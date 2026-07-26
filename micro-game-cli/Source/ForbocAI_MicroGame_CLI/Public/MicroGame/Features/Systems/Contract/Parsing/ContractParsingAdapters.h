#pragma once

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "MicroGame/Features/Systems/Contract/Parsing/Scenario/ScenarioAdapters.h"

namespace MicroGame::Contract {

/** User Story: As a contract consumer, I need API JSON decoded into executable scenarios and API-owned coverage groups atomically so malformed data cannot enter the store. @fn inline FContractResponse ParseContractJson(const FString &JsonBody) */
inline FContractResponse ParseContractJson(const FString &JsonBody) {
  FContractResponse Response;
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(JsonBody);
  return !FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()
             ? Response
             : [&]() {
                 const TArray<TSharedPtr<FJsonValue>> *Groups = nullptr;
                 const TArray<TSharedPtr<FJsonValue>> *Scenarios = nullptr;
                 const TSharedPtr<FJsonObject> *Aliases = nullptr;
                 const FContractSchemaData &Schema = ContractData().Schema;
                 return !Root->TryGetStringField(Schema.version,
                                                 Response.Version) ||
                                !Root->TryGetStringField(
                                    Schema.slotContractVersion,
                                    Response.SlotContractVersion) ||
                                !Root->TryGetArrayField(
                                    Schema.requiredCommandGroups, Groups) ||
                                !Root->TryGetObjectField(Schema.aliasRules,
                                                        Aliases) ||
                                !Root->TryGetArrayField(Schema.scenarios,
                                                       Scenarios)
                            ? Response
                            : [&]() {
                                const Parsing::TParseResult<TArray<FString>>
                                    ParsedGroups = Parsing::ParseStrings(
                                        *Groups,
                                        VerificationVocabularyAdapters::GameRuntimeData()
                                            .numbers.emptyCount,
                                        {});
                                Parsing::TParseResult<TArray<FScenarioStep>>
                                    ParsedScenarios = Parsing::ParseScenarios(
                                        *Scenarios,
                                        VerificationVocabularyAdapters::GameRuntimeData()
                                            .numbers.emptyCount,
                                        {});
                                const bool bAliasesValid =
                                    (*Aliases)->TryGetStringField(
                                        Schema.npcCreateAlias,
                                        Response.AliasRules.NpcCreateAlias) &&
                                    (*Aliases)->TryGetStringField(
                                        Schema.ghostSessionAlias,
                                        Response.AliasRules
                                            .GhostSessionAlias) &&
                                    (*Aliases)->TryGetStringField(
                                        Schema.soulTransactionAlias,
                                        Response.AliasRules
                                            .SoulTransactionAlias);
                                Response.bValid =
                                    !Response.Version.IsEmpty() &&
                                    !Response.SlotContractVersion.IsEmpty() &&
                                    ParsedGroups.bValid &&
                                    ParsedGroups.Parsed.Num() >
                                        VerificationVocabularyAdapters::GameRuntimeData()
                                            .numbers.emptyCount &&
                                    Parsing::HasKnownRequiredGroups(
                                        ParsedGroups.Parsed,
                                        VerificationVocabularyAdapters::GameRuntimeData()
                                            .numbers.emptyCount) &&
                                    bAliasesValid &&
                                    !Response.AliasRules.NpcCreateAlias
                                         .IsEmpty() &&
                                    !Response.AliasRules.GhostSessionAlias
                                         .IsEmpty() &&
                                    !Response.AliasRules.SoulTransactionAlias
                                         .IsEmpty() &&
                                    ParsedScenarios.bValid &&
                                    ParsedScenarios.Parsed.Num() >
                                        VerificationVocabularyAdapters::GameRuntimeData()
                                            .numbers.emptyCount;
                                return !Response.bValid
                                           ? FContractResponse()
                                           : (Response.RequiredCommandGroups =
                                                  ParsedGroups.Parsed,
                                              Response.Scenarios = MoveTemp(
                                                  ParsedScenarios.Parsed),
                                              Response);
                              }();
               }();
}

} // namespace MicroGame::Contract
