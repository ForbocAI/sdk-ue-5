#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace Contract {
namespace detail {

/** User Story: As a systems contract consumer, I need command groups decoded from API-owned names through a stable signature so unknown groups invalidate the contract. @fn inline ECommandGroup ParseCommandGroup(const FString &Group) */
inline ECommandGroup ParseCommandGroup(const FString &Group) {
  return Group == TEXT("status")              ? ECommandGroup::Status
       : Group == TEXT("npc_lifecycle")       ? ECommandGroup::NpcLifecycle
       : Group == TEXT("npc_process_chat")    ? ECommandGroup::NpcProcessChat
       : Group == TEXT("memory_list")         ? ECommandGroup::listMemory
       : Group == TEXT("memory_recall")       ? ECommandGroup::recallMemory
       : Group == TEXT("memory_store")        ? ECommandGroup::storeMemory
       : Group == TEXT("memory_clear")        ? ECommandGroup::clearMemory
       : Group == TEXT("memory_export")       ? ECommandGroup::MemoryExport
       : Group == TEXT("bridge_rules")        ? ECommandGroup::getBridgeRules
       : Group == TEXT("bridge_validate")     ? ECommandGroup::BridgeValidate
       : Group == TEXT("bridge_preset")       ? ECommandGroup::loadBridgePreset
       : Group == TEXT("soul_export")         ? ECommandGroup::SoulExport
       : Group == TEXT("soul_import")         ? ECommandGroup::SoulImport
       : Group == TEXT("soul_list")           ? ECommandGroup::SoulList
       : Group == TEXT("soul_chat")           ? ECommandGroup::SoulChat
       : Group == TEXT("ghost_lifecycle")     ? ECommandGroup::GhostLifecycle
                                                : ECommandGroup::Unknown;
}

/** User Story: As a systems contract consumer, I need event types decoded from API-owned names through a stable signature so unknown events invalidate the contract. @fn inline EEventType ParseEventType(const FString &EventType) */
inline EEventType ParseEventType(const FString &EventType) {
  return EventType == TEXT("stealth")     ? EEventType::Stealth
       : EventType == TEXT("social")      ? EEventType::Social
       : EventType == TEXT("escape")      ? EEventType::Escape
       : EventType == TEXT("persistence") ? EEventType::Persistence
                                            : EEventType::Unknown;
}

/** User Story: As a systems contract consumer, I need output assertion kinds decoded from API-owned names through a stable signature so unknown semantics invalidate the contract. @fn inline EOutputAssertionKind ParseOutputAssertionKind(const FString &Kind) */
inline EOutputAssertionKind ParseOutputAssertionKind(const FString &Kind) {
  return Kind == TEXT("includesAlias")
             ? EOutputAssertionKind::IncludesAlias
         : Kind == TEXT("includesText")
             ? EOutputAssertionKind::IncludesText
             : EOutputAssertionKind::Unknown;
}

/**
 * User Story: As a systems contract consumer, I need output assertions validated recursively through a stable signature so no assertion is ignored.
 * @fn inline bool HasKnownOutputAssertions( const TArray<FContractOutputAssertion> &Assertions, int32 Index)
 */
inline bool HasKnownOutputAssertions(
    const TArray<FContractOutputAssertion> &Assertions, int32 Index) {
  return Index >= Assertions.Num()
             ? true
             : !Assertions[Index].Value.IsEmpty() &&
                   ParseOutputAssertionKind(Assertions[Index].Kind) !=
                       EOutputAssertionKind::Unknown &&
                   HasKnownOutputAssertions(Assertions, Index + 1);
}

/**
 * User Story: As a systems contract consumer, I need command groups validated recursively through a stable signature so every command carries known semantics.
 * @fn inline bool HasKnownCommandGroups( const TArray<FContractCommandSpec> &Commands, int32 Index)
 */
inline bool HasKnownCommandGroups(
    const TArray<FContractCommandSpec> &Commands, int32 Index) {
  return Index >= Commands.Num()
             ? true
             : ParseCommandGroup(Commands[Index].Group) !=
                       ECommandGroup::Unknown &&
                   Commands[Index].bHasOutputAssertions &&
                   HasKnownOutputAssertions(
                       Commands[Index].OutputAssertions, 0) &&
                   HasKnownCommandGroups(Commands, Index + 1);
}

/** User Story: As a systems contract consumer, I need scenarios validated recursively through a stable signature so every event and command is known. @fn inline bool HasKnownScenarios(const TArray<FContractScenario> &Scenarios, int32 Index) */
inline bool HasKnownScenarios(const TArray<FContractScenario> &Scenarios,
                              int32 Index) {
  return Index >= Scenarios.Num()
             ? true
             : ParseEventType(Scenarios[Index].EventType) !=
                       EEventType::Unknown &&
                   HasKnownCommandGroups(Scenarios[Index].Commands, 0) &&
                   HasKnownScenarios(Scenarios, Index + 1);
}

/** User Story: As a systems contract consumer, I need required groups validated recursively through a stable signature so unknown coverage claims invalidate the contract. @fn inline bool HasKnownRequiredGroups(const TArray<FString> &Groups, int32 Index) */
inline bool HasKnownRequiredGroups(const TArray<FString> &Groups,
                                   int32 Index) {
  return Index >= Groups.Num()
             ? true
             : ParseCommandGroup(Groups[Index]) != ECommandGroup::Unknown &&
                   HasKnownRequiredGroups(Groups, Index + 1);
}

/**
 * User Story: As a systems contract consumer, I need JSON string arrays parsed recursively through a stable signature so API ordering remains deterministic.
 * @fn inline TArray<FString> ParseStrings( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FString> Acc)
 */
inline TArray<FString> ParseStrings(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FString> Acc) {
  return Index >= Values.Num()
             ? Acc
             : (Acc.Add(Values[Index]->AsString()),
                ParseStrings(Values, Index + 1, MoveTemp(Acc)));
}

/**
 * User Story: As a systems contract consumer, I need output assertions parsed recursively through a stable signature so API order and semantics remain deterministic.
 * @fn inline TArray<FContractOutputAssertion> ParseOutputAssertions( const TArray<TSharedPtr<FJsonValue>> &Assertions, int32 Index, TArray<FContractOutputAssertion> Acc)
 */
inline TArray<FContractOutputAssertion> ParseOutputAssertions(
    const TArray<TSharedPtr<FJsonValue>> &Assertions, int32 Index,
    TArray<FContractOutputAssertion> Acc) {
  return Index >= Assertions.Num()
             ? Acc
             : [&]() {
                 const TSharedPtr<FJsonObject> Object =
                     Assertions[Index]->AsObject();
                 FContractOutputAssertion Assertion;
                 Assertion.Kind = Object->GetStringField(TEXT("kind"));
                 Assertion.Value = Object->GetStringField(TEXT("value"));
                 Acc.Add(MoveTemp(Assertion));
                 return ParseOutputAssertions(Assertions, Index + 1,
                                              MoveTemp(Acc));
               }();
}

/**
 * User Story: As a systems contract consumer, I need commands parsed recursively through a stable signature so routes and assertions remain attached to their command.
 * @fn inline TArray<FContractCommandSpec> ParseCommands( const TArray<TSharedPtr<FJsonValue>> &Commands, int32 Index, TArray<FContractCommandSpec> Acc)
 */
inline TArray<FContractCommandSpec> ParseCommands(
    const TArray<TSharedPtr<FJsonValue>> &Commands, int32 Index,
    TArray<FContractCommandSpec> Acc) {
  return Index >= Commands.Num()
             ? Acc
             : [&]() {
                 const TSharedPtr<FJsonObject> Object =
                     Commands[Index]->AsObject();
                 FContractCommandSpec Command;
                 Command.Group = Object->GetStringField(TEXT("group"));
                 Command.Command = Object->GetStringField(TEXT("command"));
                 const TArray<TSharedPtr<FJsonValue>> *Routes = nullptr;
                 Object->TryGetArrayField(TEXT("expectedRoutes"), Routes)
                     ? (Command.ExpectedRoutes = ParseStrings(*Routes, 0, {}),
                        void())
                     : void();
                 const TArray<TSharedPtr<FJsonValue>> *Assertions = nullptr;
                 Command.bHasOutputAssertions = Object->TryGetArrayField(
                     TEXT("outputAssertions"), Assertions);
                 Command.bHasOutputAssertions
                     ? (Command.OutputAssertions = ParseOutputAssertions(
                            *Assertions, 0, {}),
                        void())
                     : void();
                 Acc.Add(MoveTemp(Command));
                 return ParseCommands(Commands, Index + 1, MoveTemp(Acc));
               }();
}

/**
 * User Story: As a systems contract consumer, I need scenarios parsed recursively through a stable signature so the complete API contract becomes typed harness state.
 * @fn inline TArray<FContractScenario> ParseScenarios( const TArray<TSharedPtr<FJsonValue>> &Scenarios, int32 Index, TArray<FContractScenario> Acc)
 */
inline TArray<FContractScenario> ParseScenarios(
    const TArray<TSharedPtr<FJsonValue>> &Scenarios, int32 Index,
    TArray<FContractScenario> Acc) {
  return Index >= Scenarios.Num()
             ? Acc
             : [&]() {
                 const TSharedPtr<FJsonObject> Object =
                     Scenarios[Index]->AsObject();
                 FContractScenario Scenario;
                 Scenario.Id = Object->GetStringField(TEXT("id"));
                 Scenario.Title = Object->GetStringField(TEXT("title"));
                 Scenario.Description =
                     Object->GetStringField(TEXT("description"));
                 Scenario.EventType =
                     Object->GetStringField(TEXT("eventType"));
                 const TArray<TSharedPtr<FJsonValue>> *Commands = nullptr;
                 Object->TryGetArrayField(TEXT("commands"), Commands)
                     ? (Scenario.Commands = ParseCommands(*Commands, 0, {}),
                        void())
                     : void();
                 Acc.Add(MoveTemp(Scenario));
                 return ParseScenarios(Scenarios, Index + 1, MoveTemp(Acc));
               }();
}

} // namespace detail
} // namespace Contract
} // namespace TestGame
