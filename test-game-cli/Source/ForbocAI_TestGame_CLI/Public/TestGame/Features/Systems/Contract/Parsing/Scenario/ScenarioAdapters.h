#pragma once

#include "TestGame/Features/Systems/Contract/Parsing/Command/CommandAdapters.h"

namespace TestGame::Contract::Parsing {

/** User Story: As a contract consumer, I need one scenario decoded directly into the root-store shape so no adapter can reinterpret API semantics. @fn inline TParseResult<FScenarioStep> ParseScenario(const TSharedPtr<FJsonValue> &Value) */
inline TParseResult<FScenarioStep>
ParseScenario(const TSharedPtr<FJsonValue> &Value) {
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject()
                                                     : nullptr;
  FScenarioStep Scenario;
  const TArray<TSharedPtr<FJsonValue>> *Commands = nullptr;
  return !Object.IsValid() ||
                 !Object->TryGetStringField(ContractData().Schema.id,
                                            Scenario.Id) ||
                 !Object->TryGetStringField(ContractData().Schema.title,
                                            Scenario.Title) ||
                 !Object->TryGetStringField(ContractData().Schema.description,
                                            Scenario.Description) ||
                 !Object->TryGetStringField(ContractData().Schema.eventType,
                                            Scenario.EventType) ||
                 !Object->TryGetArrayField(ContractData().Schema.commands,
                                           Commands)
             ? TParseResult<FScenarioStep>()
             : [&]() {
                 TParseResult<TArray<FCommandSpec>> ParsedCommands =
                     ParseCommands(
                         *Commands,
                         GameAdapters::GameRuntimeData().numbers.emptyCount,
                         {});
                 return Scenario.Id.IsEmpty() || Scenario.Title.IsEmpty() ||
                                !GameAdapters::GameRuntimeData()
                                     .scenarioEventTypes.all.Contains(
                                         Scenario.EventType) ||
                                !ParsedCommands.bValid
                            ? TParseResult<FScenarioStep>()
                            : (Scenario.Commands =
                                   MoveTemp(ParsedCommands.Parsed),
                               TParseResult<FScenarioStep>{
                                   true, MoveTemp(Scenario)});
               }();
}

/** User Story: As a contract consumer, I need every scenario decoded recursively so malformed API responses fail atomically before dispatch. @fn inline TParseResult<TArray<FScenarioStep>> ParseScenarios( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FScenarioStep> Acc) */
inline TParseResult<TArray<FScenarioStep>> ParseScenarios(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FScenarioStep> Acc) {
  return Index >= Values.Num()
             ? TParseResult<TArray<FScenarioStep>>{true, MoveTemp(Acc)}
             : [&]() {
                 TParseResult<FScenarioStep> Scenario =
                     ParseScenario(Values[Index]);
                 return !Scenario.bValid
                            ? TParseResult<TArray<FScenarioStep>>()
                            : (Acc.Add(MoveTemp(Scenario.Parsed)),
                               ParseScenarios(
                                   Values,
                                   Index + GameAdapters::GameRuntimeData()
                                               .numbers.nextIndex,
                                   MoveTemp(Acc)));
               }();
}

/** User Story: As a contract consumer, I need every required group checked against authored runtime vocabulary so unknown API coverage claims cannot pass. @fn inline bool HasKnownRequiredGroups(const TArray<FString> &Groups, int32 Index) */
inline bool HasKnownRequiredGroups(const TArray<FString> &Groups,
                                   int32 Index) {
  return Index >= Groups.Num()
             ? true
             : GameAdapters::GameRuntimeData()
                       .commandGroups.all.Contains(Groups[Index]) &&
                   HasKnownRequiredGroups(
                       Groups, Index + GameAdapters::GameRuntimeData()
                                           .numbers.nextIndex);
}

} // namespace TestGame::Contract::Parsing
