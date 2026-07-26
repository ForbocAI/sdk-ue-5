#pragma once

#include "MicroGame/Features/Systems/Contract/ContractAdapters.h"
#include "MicroGame/Features/Systems/Contract/Parsing/ParsingTypes.h"
#include "MicroGame/Features/Systems/Harness/Verification/Vocabulary/VerificationVocabularyAdapters.h"

namespace MicroGame::Contract::Parsing {

/** User Story: As a contract consumer, I need JSON string arrays decoded recursively so malformed route or coverage values invalidate the whole response. @fn inline TParseResult<TArray<FString>> ParseStrings( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FString> Acc) */
inline TParseResult<TArray<FString>> ParseStrings(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FString> Acc) {
  return Index >= Values.Num()
             ? TParseResult<TArray<FString>>{true, MoveTemp(Acc)}
             : [&]() {
                 FString Value;
                 return !Values[Index].IsValid() ||
                                !Values[Index]->TryGetString(Value)
                            ? TParseResult<TArray<FString>>()
                            : (Acc.Add(MoveTemp(Value)),
                               ParseStrings(
                                   Values,
                                   Index + VerificationVocabularyAdapters::GameRuntimeData()
                                               .numbers.nextIndex,
                                   MoveTemp(Acc)));
               }();
}

/** User Story: As a contract consumer, I need one semantic output assertion decoded without conversion tables so API-owned names remain authoritative. @fn inline TParseResult<FOutputAssertion> ParseOutputAssertion(const TSharedPtr<FJsonValue> &Value) */
inline TParseResult<FOutputAssertion>
ParseOutputAssertion(const TSharedPtr<FJsonValue> &Value) {
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject()
                                                     : nullptr;
  FOutputAssertion Assertion;
  return !Object.IsValid() ||
                 !Object->TryGetStringField(ContractData().Schema.kind,
                                            Assertion.Kind) ||
                 !Object->TryGetStringField(ContractData().Schema.value,
                                            Assertion.Value) ||
                 Assertion.Value.IsEmpty() ||
                 !VerificationVocabularyAdapters::GameRuntimeData()
                      .outputAssertionKinds.all.Contains(Assertion.Kind)
             ? TParseResult<FOutputAssertion>()
             : TParseResult<FOutputAssertion>{true, MoveTemp(Assertion)};
}

/** User Story: As a contract consumer, I need every command assertion decoded recursively so one unknown assertion invalidates the complete command. @fn inline TParseResult<TArray<FOutputAssertion>> ParseOutputAssertions( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FOutputAssertion> Acc) */
inline TParseResult<TArray<FOutputAssertion>> ParseOutputAssertions(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FOutputAssertion> Acc) {
  return Index >= Values.Num()
             ? TParseResult<TArray<FOutputAssertion>>{true, MoveTemp(Acc)}
             : [&]() {
                 TParseResult<FOutputAssertion> Assertion =
                     ParseOutputAssertion(Values[Index]);
                 return !Assertion.bValid
                            ? TParseResult<TArray<FOutputAssertion>>()
                            : (Acc.Add(MoveTemp(Assertion.Parsed)),
                               ParseOutputAssertions(
                                   Values,
                                   Index + VerificationVocabularyAdapters::GameRuntimeData()
                                               .numbers.nextIndex,
                                   MoveTemp(Acc)));
               }();
}

/** User Story: As a contract consumer, I need one command decoded directly into the executable harness type so transport and runtime cannot drift. @fn inline TParseResult<FCommandSpec> ParseCommand(const TSharedPtr<FJsonValue> &Value) */
inline TParseResult<FCommandSpec>
ParseCommand(const TSharedPtr<FJsonValue> &Value) {
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject()
                                                     : nullptr;
  FCommandSpec Command;
  const TArray<TSharedPtr<FJsonValue>> *Routes = nullptr;
  const TArray<TSharedPtr<FJsonValue>> *Assertions = nullptr;
  return !Object.IsValid() ||
                 !Object->TryGetStringField(ContractData().Schema.group,
                                            Command.Group) ||
                 !Object->TryGetStringField(ContractData().Schema.command,
                                            Command.Command) ||
                 !Object->TryGetArrayField(
                     ContractData().Schema.expectedRoutes, Routes) ||
                 !Object->TryGetArrayField(
                     ContractData().Schema.outputAssertions, Assertions)
             ? TParseResult<FCommandSpec>()
             : [&]() {
                 const TParseResult<TArray<FString>> ParsedRoutes =
                     ParseStrings(
                         *Routes,
                         VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount,
                         {});
                 const TParseResult<TArray<FOutputAssertion>>
                     ParsedAssertions = ParseOutputAssertions(
                         *Assertions,
                         VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount,
                         {});
                 return Command.Command.IsEmpty() ||
                                !VerificationVocabularyAdapters::GameRuntimeData()
                                     .commandGroups.all.Contains(
                                         Command.Group) ||
                                !ParsedRoutes.bValid ||
                                !ParsedAssertions.bValid
                            ? TParseResult<FCommandSpec>()
                            : [&]() {
                                Command.ExpectedRoutes = ParsedRoutes.Parsed;
                                Command.OutputAssertions =
                                    ParsedAssertions.Parsed;
                                return TParseResult<FCommandSpec>{
                                    true, MoveTemp(Command)};
                              }();
               }();
}

/** User Story: As a contract consumer, I need every scenario command decoded recursively so partial command lists never enter the root store. @fn inline TParseResult<TArray<FCommandSpec>> ParseCommands( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FCommandSpec> Acc) */
inline TParseResult<TArray<FCommandSpec>> ParseCommands(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FCommandSpec> Acc) {
  return Index >= Values.Num()
             ? TParseResult<TArray<FCommandSpec>>{true, MoveTemp(Acc)}
             : [&]() {
                 TParseResult<FCommandSpec> Command =
                     ParseCommand(Values[Index]);
                 return !Command.bValid
                            ? TParseResult<TArray<FCommandSpec>>()
                            : (Acc.Add(MoveTemp(Command.Parsed)),
                               ParseCommands(
                                   Values,
                                   Index + VerificationVocabularyAdapters::GameRuntimeData()
                                               .numbers.nextIndex,
                                   MoveTemp(Acc)));
               }();
}

} // namespace MicroGame::Contract::Parsing
