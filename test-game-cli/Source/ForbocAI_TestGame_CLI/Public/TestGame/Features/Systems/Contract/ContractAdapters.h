#pragma once

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"
#include "TestGame/Features/Systems/Harness/Commands/CommandsTypes.h"
#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace Contract {

namespace detail {

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

inline EEventType ParseEventType(const FString &EventType) {
  return EventType == TEXT("stealth")     ? EEventType::Stealth
       : EventType == TEXT("social")      ? EEventType::Social
       : EventType == TEXT("escape")      ? EEventType::Escape
       : EventType == TEXT("persistence") ? EEventType::Persistence
                                            : EEventType::Unknown;
}

inline bool HasKnownCommandGroups(
    const TArray<FContractCommandSpec> &Commands, int32 Index) {
  return Index >= Commands.Num()
             ? true
             : ParseCommandGroup(Commands[Index].Group) !=
                       ECommandGroup::Unknown &&
                   HasKnownCommandGroups(Commands, Index + 1);
}

inline bool HasKnownScenarios(const TArray<FContractScenario> &Scenarios,
                              int32 Index) {
  return Index >= Scenarios.Num()
             ? true
             : ParseEventType(Scenarios[Index].EventType) !=
                       EEventType::Unknown &&
                   HasKnownCommandGroups(Scenarios[Index].Commands, 0) &&
                   HasKnownScenarios(Scenarios, Index + 1);
}

inline bool HasKnownRequiredGroups(const TArray<FString> &Groups,
                                   int32 Index) {
  return Index >= Groups.Num()
             ? true
             : ParseCommandGroup(Groups[Index]) != ECommandGroup::Unknown &&
                   HasKnownRequiredGroups(Groups, Index + 1);
}

inline TArray<FString> ParseStrings(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FString> Acc) {
  return Index >= Values.Num()
             ? Acc
             : (Acc.Add(Values[Index]->AsString()),
                ParseStrings(Values, Index + 1, MoveTemp(Acc)));
}

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
                 Acc.Add(MoveTemp(Command));
                 return ParseCommands(Commands, Index + 1, MoveTemp(Acc));
               }();
}

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

inline FCommandSpec ToCommandSpec(const FContractCommandSpec &Command) {
  FCommandSpec Result;
  Result.Group = detail::ParseCommandGroup(Command.Group);
  Result.Command = Command.Command;
  Result.ExpectedRoutes = Command.ExpectedRoutes;
  return Result;
}

inline TArray<FCommandSpec> ToCommandSpecs(
    const TArray<FContractCommandSpec> &Commands, int32 Index,
    TArray<FCommandSpec> Acc) {
  return Index >= Commands.Num()
             ? Acc
             : (Acc.Add(ToCommandSpec(Commands[Index])),
                ToCommandSpecs(Commands, Index + 1, MoveTemp(Acc)));
}

inline FScenarioStep ToScenarioStep(const FContractScenario &Scenario) {
  FScenarioStep Result;
  Result.Id = Scenario.Id;
  Result.Title = Scenario.Title;
  Result.Description = Scenario.Description;
  Result.EventType = detail::ParseEventType(Scenario.EventType);
  Result.Commands = ToCommandSpecs(Scenario.Commands, 0, {});
  return Result;
}

inline TArray<FScenarioStep> ToScenarioSteps(
    const TArray<FContractScenario> &Scenarios, int32 Index,
    TArray<FScenarioStep> Acc) {
  return Index >= Scenarios.Num()
             ? Acc
             : (Acc.Add(ToScenarioStep(Scenarios[Index])),
                ToScenarioSteps(Scenarios, Index + 1, MoveTemp(Acc)));
}

inline FContractResponse ParseContractJson(const FString &JsonBody) {
  FContractResponse Response;
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(JsonBody);
  return !FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()
             ? Response
             : [&]() {
                 Response.Version = Root->GetStringField(TEXT("version"));
                 Response.SlotContractVersion =
                     Root->GetStringField(TEXT("slotContractVersion"));

                 const TArray<TSharedPtr<FJsonValue>> *Groups = nullptr;
                 Root->TryGetArrayField(TEXT("requiredCommandGroups"), Groups)
                     ? (Response.RequiredCommandGroups =
                            detail::ParseStrings(*Groups, 0, {}),
                        void())
                     : void();

                 const TSharedPtr<FJsonObject> *Aliases = nullptr;
                 Root->TryGetObjectField(TEXT("aliasRules"), Aliases)
                     ? (Response.AliasRules.NpcCreateAlias =
                            (*Aliases)->GetStringField(TEXT("npcCreateAlias")),
                        Response.AliasRules.BridgeValidateCommand =
                            (*Aliases)->GetStringField(
                                TEXT("bridgeValidateCommand")),
                        void())
                     : void();

                 const TArray<TSharedPtr<FJsonValue>> *Scenarios = nullptr;
                 Root->TryGetArrayField(TEXT("scenarios"), Scenarios)
                     ? (Response.Scenarios =
                            detail::ParseScenarios(*Scenarios, 0, {}),
                        void())
                     : void();
                 Response.bValid =
                     !Response.Version.IsEmpty() &&
                     !Response.SlotContractVersion.IsEmpty() &&
                     Response.RequiredCommandGroups.Num() > 0 &&
                     Response.Scenarios.Num() > 0 &&
                     detail::HasKnownRequiredGroups(
                         Response.RequiredCommandGroups, 0) &&
                     detail::HasKnownScenarios(Response.Scenarios, 0);
                 return Response;
               }();
}

inline TArray<ECommandGroup> ToCommandGroups(
    const TArray<FString> &Groups, int32 Index,
    TArray<ECommandGroup> Acc) {
  return Index >= Groups.Num()
             ? Acc
             : (Acc.Add(detail::ParseCommandGroup(Groups[Index])),
                ToCommandGroups(Groups, Index + 1, MoveTemp(Acc)));
}

} // namespace Contract
} // namespace TestGame
