#pragma once

#include "Core/rtk.hpp"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Contract/Parsing/ParsingAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace Contract {

/** User Story: As a features systems contract consumer, I need to invoke contract settings source through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline const DataAdapters::FSettingsSource &ContractSettingsSource() */
inline const DataAdapters::FSettingsSource &ContractSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/contract.json"));
  return Source;
}

/** User Story: As a features systems contract consumer, I need to invoke resolve test game api url through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FString resolveTestGameApiUrl(const FString &ApiUrl) */
inline FString resolveTestGameApiUrl(const FString &ApiUrl) {
  const DataAdapters::FSettingsSource &Source = ContractSettingsSource();
  const TSharedRef<FJsonObject> Separators =
      DataAdapters::ReadObjectField(Source, TEXT("separators"));
  FString Resolved = ApiUrl.TrimStartAndEnd();
  Resolved = Resolved.IsEmpty()
                 ? DataAdapters::ReadStringField(Source.Root,
                                                 TEXT("defaultApiUrl"))
                 : Resolved;
  Resolved.RemoveFromEnd(
      DataAdapters::ReadStringField(Separators, TEXT("trailingUrl")));
  return Resolved;
}

/** User Story: As a features systems contract consumer, I need to invoke create test game auth headers through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TMap<FString, FString> createTestGameAuthHeaders(const FString &ApiKey) */
inline TMap<FString, FString>
createTestGameAuthHeaders(const FString &ApiKey) {
  return func::match(
      func::fromNullable(ApiKey, !ApiKey.IsEmpty()),
      [](const FString &ValidApiKey) {
        const TSharedRef<FJsonObject> Authorization =
            DataAdapters::ReadObjectField(ContractSettingsSource(),
                                          TEXT("authorization"));
        const FString Header = DataAdapters::ReadStringField(
            Authorization, TEXT("header"));
        const FString Value = FString::Format(
            *DataAdapters::ReadStringField(Authorization, TEXT("template")),
            {ValidApiKey});
        return func::upsert_map_value<FString, FString>(
            TMap<FString, FString>(), Header, FString(),
            [Value](const FString &) { return Value; });
      },
      []() { return TMap<FString, FString>(); });
}

struct FTestGameContractRequest {
  rtk::FetchArgs Args;
  TMap<FString, FString> Headers;
};

/** User Story: As a features systems contract consumer, I need to invoke create test game contract request through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FTestGameContractRequest createTestGameContractRequest( const FString &ApiUrl, const FString &ApiKey) */
inline FTestGameContractRequest createTestGameContractRequest(
    const FString &ApiUrl, const FString &ApiKey) {
  const TSharedRef<FJsonObject> Request = DataAdapters::ReadObjectField(
      ContractSettingsSource(), TEXT("request"));
  FTestGameContractRequest Result;
  Result.Args.url = resolveTestGameApiUrl(ApiUrl) +
                    DataAdapters::ReadStringField(Request, TEXT("path"));
  Result.Args.method =
      DataAdapters::ReadStringField(Request, TEXT("method"));
  Result.Headers = createTestGameAuthHeaders(ApiKey);
  return Result;
}

/** User Story: As a systems contract consumer, I need contract assertions converted into executable harness assertions through a stable signature so both representations share one semantic model. @fn inline FOutputAssertion ToOutputAssertion(const FContractOutputAssertion &Assertion) */
inline FOutputAssertion
ToOutputAssertion(const FContractOutputAssertion &Assertion) {
  FOutputAssertion Result;
  Result.Kind = detail::ParseOutputAssertionKind(Assertion.Kind);
  Result.Value = Assertion.Value;
  return Result;
}

/**
 * User Story: As a systems contract consumer, I need assertion arrays converted recursively through a stable signature so ordering remains stable across hosts.
 * @fn inline TArray<FOutputAssertion> ToOutputAssertions( const TArray<FContractOutputAssertion> &Assertions, int32 Index, TArray<FOutputAssertion> Acc)
 */
inline TArray<FOutputAssertion> ToOutputAssertions(
    const TArray<FContractOutputAssertion> &Assertions, int32 Index,
    TArray<FOutputAssertion> Acc) {
  return Index >= Assertions.Num()
             ? Acc
             : (Acc.Add(ToOutputAssertion(Assertions[Index])),
                ToOutputAssertions(Assertions, Index + 1, MoveTemp(Acc)));
}

/** User Story: As a features systems contract consumer, I need to invoke to command spec through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FCommandSpec ToCommandSpec(const FContractCommandSpec &Command) */
inline FCommandSpec ToCommandSpec(const FContractCommandSpec &Command) {
  FCommandSpec Result;
  Result.Group = detail::ParseCommandGroup(Command.Group);
  Result.Command = Command.Command;
  Result.ExpectedRoutes = Command.ExpectedRoutes;
  Result.OutputAssertions =
      ToOutputAssertions(Command.OutputAssertions, 0, {});
  return Result;
}

/** User Story: As a features systems contract consumer, I need to invoke to command specs through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TArray<FCommandSpec> ToCommandSpecs( const TArray<FContractCommandSpec> &Commands, int32 Index, TArray<FCommandSpec> Acc) */
inline TArray<FCommandSpec> ToCommandSpecs(
    const TArray<FContractCommandSpec> &Commands, int32 Index,
    TArray<FCommandSpec> Acc) {
  return Index >= Commands.Num()
             ? Acc
             : (Acc.Add(ToCommandSpec(Commands[Index])),
                ToCommandSpecs(Commands, Index + 1, MoveTemp(Acc)));
}

/** User Story: As a features systems contract consumer, I need to invoke to scenario step through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FScenarioStep ToScenarioStep(const FContractScenario &Scenario) */
inline FScenarioStep ToScenarioStep(const FContractScenario &Scenario) {
  FScenarioStep Result;
  Result.Id = Scenario.Id;
  Result.Title = Scenario.Title;
  Result.Description = Scenario.Description;
  Result.EventType = detail::ParseEventType(Scenario.EventType);
  Result.Commands = ToCommandSpecs(Scenario.Commands, 0, {});
  return Result;
}

/** User Story: As a features systems contract consumer, I need to invoke to scenario steps through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TArray<FScenarioStep> ToScenarioSteps( const TArray<FContractScenario> &Scenarios, int32 Index, TArray<FScenarioStep> Acc) */
inline TArray<FScenarioStep> ToScenarioSteps(
    const TArray<FContractScenario> &Scenarios, int32 Index,
    TArray<FScenarioStep> Acc) {
  return Index >= Scenarios.Num()
             ? Acc
             : (Acc.Add(ToScenarioStep(Scenarios[Index])),
                ToScenarioSteps(Scenarios, Index + 1, MoveTemp(Acc)));
}

/** User Story: As a features systems contract consumer, I need to invoke parse contract json through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FContractResponse ParseContractJson(const FString &JsonBody) */
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
                        Response.AliasRules.GhostSessionAlias =
                            (*Aliases)->GetStringField(TEXT("ghostSessionAlias")),
                        Response.AliasRules.SoulTransactionAlias =
                            (*Aliases)->GetStringField(TEXT("soulTransactionAlias")),
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
                     !Response.AliasRules.NpcCreateAlias.IsEmpty() &&
                     !Response.AliasRules.GhostSessionAlias.IsEmpty() &&
                     !Response.AliasRules.SoulTransactionAlias.IsEmpty() &&
                     Response.Scenarios.Num() > 0 &&
                     detail::HasKnownRequiredGroups(
                         Response.RequiredCommandGroups, 0) &&
                     detail::HasKnownScenarios(Response.Scenarios, 0);
                 return Response;
               }();
}

/** User Story: As a features systems contract consumer, I need to invoke to command groups through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TArray<ECommandGroup> ToCommandGroups( const TArray<FString> &Groups, int32 Index, TArray<ECommandGroup> Acc) */
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
