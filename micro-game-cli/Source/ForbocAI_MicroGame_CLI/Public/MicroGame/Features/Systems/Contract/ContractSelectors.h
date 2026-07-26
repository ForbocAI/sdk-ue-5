#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Contract/ContractTypes.h"
#include "MicroGame/Features/Systems/Harness/Verification/Vocabulary/VerificationVocabularyAdapters.h"

namespace MicroGame::ContractSelectors {

/** User Story: As a contract verifier, I need output assertions compared recursively so partial final commands cannot satisfy coverage. @fn inline bool AssertionsMatch(const TArray<FOutputAssertion> &Left, const TArray<FOutputAssertion> &Right, int32 Index) */
inline bool AssertionsMatch(const TArray<FOutputAssertion> &Left,
                            const TArray<FOutputAssertion> &Right,
                            int32 Index) {
  return Left.Num() != Right.Num()
             ? false
         : Index >= Left.Num()
             ? true
         : Left[Index].Kind != Right[Index].Kind ||
                   Left[Index].Value != Right[Index].Value
             ? false
             : AssertionsMatch(
                   Left, Right,
                   Index + VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex);
}

/** User Story: As a contract verifier, I need command identity include routes and assertions so stale API scenarios remain visible. @fn inline bool CommandMatches(const FCommandSpec &Left, const FCommandSpec &Right) */
inline bool CommandMatches(const FCommandSpec &Left,
                           const FCommandSpec &Right) {
  return Left.Group == Right.Group && Left.Command == Right.Command &&
         Left.ExpectedRoutes == Right.ExpectedRoutes &&
         AssertionsMatch(
             Left.OutputAssertions, Right.OutputAssertions,
             VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount);
}

/**
 * User Story: As a release verifier, I need required final checks selected when the live API contract omits or changes them.
 * @fn inline TArray<FCommandSpec> SelectMissingFinalCommands( const Contract::FContractResponse &Contract, const TArray<FCommandSpec> &FinalCommands)
 */
inline TArray<FCommandSpec> SelectMissingFinalCommands(
    const Contract::FContractResponse &Contract,
    const TArray<FCommandSpec> &FinalCommands) {
  const TArray<FCommandSpec> ContractCommands =
      func::concat_arrays<FCommandSpec>(
          func::map_array<FScenarioStep, TArray<FCommandSpec>>(
              Contract.Scenarios, [](const FScenarioStep &Step) {
                return Step.Commands;
              }));
  return func::filter_array<FCommandSpec>(
      FinalCommands, [&ContractCommands](const FCommandSpec &FinalCommand) {
        return !func::any_array<FCommandSpec>(
            ContractCommands, [&FinalCommand](const FCommandSpec &Command) {
              return CommandMatches(Command, FinalCommand);
            });
      });
}

/**
 * User Story: As a completion selector, I need every authored final command group required even when a deployed API contract is stale.
 * @fn inline FScenarioContractPayload SelectContractWithFinalRequirements( const Contract::FContractResponse &Contract, const TArray<FCommandSpec> &FinalCommands)
 */
inline FScenarioContractPayload SelectContractWithFinalRequirements(
    const Contract::FContractResponse &Contract,
    const TArray<FCommandSpec> &FinalCommands) {
  const TArray<FString> FinalGroups =
      func::map_array<FCommandSpec, FString>(
          FinalCommands,
          [](const FCommandSpec &Command) { return Command.Group; });
  const TArray<FString> MissingGroups = func::filter_array<FString>(
      FinalGroups, [&Contract](const FString &Group) {
        return !Contract.RequiredCommandGroups.Contains(Group);
      });
  FScenarioContractPayload Payload;
  Payload.RequiredCommandGroups = func::concat_arrays<FString>(
      {Contract.RequiredCommandGroups, MissingGroups});
  Payload.Steps = Contract.Scenarios;
  return Payload;
}

} // namespace MicroGame::ContractSelectors
