#pragma once

#include "MicroGame/Features/Entities/NPCs/NPCsActions.h"
#include "MicroGame/Features/Systems/Contract/Parsing/ContractParsingAdapters.h"
#include "MicroGame/Features/Systems/Contract/ContractSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/CommandThunks.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/TwoNpcChatThunks.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/Progress/ProgressThunks.h"
#include "MicroGame/Features/Systems/Harness/Verification/Scenario/ScenarioThunks.h"
#include "MicroGame/Features/Systems/Harness/HarnessThunks.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "MicroGame/Features/Systems/Quality/QualityThunks.h"
#include "MicroGame/Features/Systems/Quality/Report/ReportAdapters.h"
#include "MicroGame/Features/Systems/Terminal/UI/UIActions.h"

namespace MicroGame {

namespace VerificationThunksDetail {

/**
 * User Story: As a release verifier, I need contract failures preserved in the transcript while independent CLI checks continue.
 * @fn inline void RecordContractFailure( const CommandRunner::FCommandOutput &ContractResult, FMicroGameStore &Store, const FGameProgressSink &Sink)
 */
inline void RecordContractFailure(
    const CommandRunner::FCommandOutput &ContractResult,
    FMicroGameStore &Store, const FGameProgressSink &Sink) {
  const FGameRuntimeData &Runtime = VerificationVocabularyAdapters::GameRuntimeData();
  CommandRunner::FCommandOutput Failure = ContractResult;
  Failure.Status = Runtime.statuses.error;
  Failure.Output = ContractResult.Status == Runtime.statuses.error &&
                           !ContractResult.Output.IsEmpty()
                       ? ContractResult.Output
                       : Runtime.messages.contractFailed;
  FScenarioStep ContractStep;
  ContractStep.Id = Runtime.commandGroups.contract;
  RecordCommandResult(ContractStep, VerificationAdapters::GameData().contractCommand,
                      Failure, Store, Sink);
  FGameProgress Progress;
  Progress.Type = Runtime.lifecycleEvents.contractFailed;
  Progress.Message = Failure.Output;
  Emit(Sink, MoveTemp(Progress));
}

/**
 * User Story: As a parity verifier, I need stale API contracts recorded as failures without suppressing required final CLI checks.
 * @fn inline void RecordContractDrift( const TArray<FCommandSpec> &MissingFinalCommands, const CommandRunner::FCommandOutput &ContractResult, FMicroGameStore &Store, const FGameProgressSink &Sink)
 */
inline void RecordContractDrift(
    const TArray<FCommandSpec> &MissingFinalCommands,
    const CommandRunner::FCommandOutput &ContractResult,
    FMicroGameStore &Store, const FGameProgressSink &Sink) {
  const FGameRuntimeData &Runtime = VerificationVocabularyAdapters::GameRuntimeData();
  const TArray<FString> MissingGroups =
      func::map_array<FCommandSpec, FString>(
          MissingFinalCommands,
          [](const FCommandSpec &Command) { return Command.Group; });
  TMap<FString, FString> Values;
  Values.Add(Runtime.tokens.groups,
             FString::Join(MissingGroups, *Runtime.separators.list));
  CommandRunner::FCommandOutput Failure = ContractResult;
  Failure.Status = Runtime.statuses.error;
  Failure.Output = VerificationAdapters::FormatGameTemplate(
      Runtime.messages.finalCommandsMissing, Values);
  FScenarioStep ContractStep;
  ContractStep.Id = Runtime.commandGroups.contract;
  RecordCommandResult(ContractStep, VerificationAdapters::GameData().contractCommand,
                      Failure, Store, Sink);
}

/** User Story: As a systems harness game consumer, I need the client-orchestrated two-NPC chat run and its transcript logged through one adapter so both the append and chat-only paths share it. @fn inline void AppendTwoNpcChat(FMicroGameStore &Store) */
inline void AppendTwoNpcChat(FMicroGameStore &Store) {
  const TArray<FString> Lines = TwoNpcChat::RunTwoNpcChat(Store);
  UE_LOG(LogTemp, Display, TEXT("%s"), *FString::Join(Lines, TEXT("\n")));
}

/** User Story: As a systems harness game consumer, I need the two-npc-chat mode to run ONLY the two-NPC chat and report a complete run so it mirrors the TS runGame early return. @fn inline FGameRunResult RunTwoNpcChatOnly(FMicroGameStore &Store) */
inline FGameRunResult RunTwoNpcChatOnly(FMicroGameStore &Store) {
  AppendTwoNpcChat(Store);
  FGameRunResult Result;
  Result.bComplete = true;
  return Result;
}

} // namespace VerificationThunksDetail

/** User Story: As a systems harness game consumer, I need one scenario run whose operations all cross the SDK CLI boundary. @fn inline FGameRunResult RunGame(FMicroGameStore &Store, FString Mode, const FGameProgressSink &ProgressSink = {}) */
inline FGameRunResult RunGame(FMicroGameStore &Store, FString Mode,
                              const FGameProgressSink &ProgressSink = {}) {
  Store.dispatch(CommandRunnerActions::aliasesReset());
  Store.dispatch(UIActions::setMode(Mode));

  // two-npc-chat runs ONLY the client-orchestrated two-NPC chat, skipping the
  // contract/scenario/quality harness (mirrors the TS runGame early return);
  // every other mode runs the full harness in the lambda below.
  return VerificationVocabularyAdapters::GameRuntimeData()
                 .twoNpcChatExclusiveModes.Contains(Mode)
      ? VerificationThunksDetail::RunTwoNpcChatOnly(Store)
      : [&]() -> FGameRunResult {

  FGameProgress Started;
  Started.Type =
      VerificationVocabularyAdapters::GameRuntimeData().lifecycleEvents.sessionStarted;
  Started.Mode = Mode;
  VerificationThunksDetail::Emit(ProgressSink, MoveTemp(Started));

  Store.dispatch(NPCsActions::UpsertNPC(
      VerificationVocabularyAdapters::GameRuntimeData().initialState.sessionNpc));

  const FGameRuntimeData &Runtime = VerificationVocabularyAdapters::GameRuntimeData();
  const FGameData &Data = VerificationAdapters::GameData();
  const CommandRunner::FCommandOutput ContractResult =
      VerificationThunksDetail::ExecuteCommand(Data.contractCommand, Store);
  const Contract::FContractResponse Contract =
      ContractResult.Status == Runtime.statuses.ok
          ? Contract::ParseContractJson(ContractResult.Output)
          : Contract::FContractResponse{};
  const bool bContractAccepted =
      ContractResult.Status == Runtime.statuses.ok && Contract.bValid;
  const TArray<FCommandSpec> MissingFinalCommands =
      bContractAccepted
          ? ContractSelectors::SelectMissingFinalCommands(
                Contract, Data.finalCommands)
          : TArray<FCommandSpec>{};
  FScenarioContractPayload ContractPayload =
      ContractSelectors::SelectContractWithFinalRequirements(
          Contract, Data.finalCommands);
  Store.dispatch(ScenarioActions::setContract(MoveTemp(ContractPayload)));
  bContractAccepted
      ? (MissingFinalCommands.Num() > Runtime.numbers.emptyCount
             ? VerificationThunksDetail::RecordContractDrift(
                   MissingFinalCommands, ContractResult, Store, ProgressSink)
             : void())
      : VerificationThunksDetail::RecordContractFailure(ContractResult, Store,
                                                ProgressSink);
  FGameProgress Ready;
  Ready.Type = Runtime.lifecycleEvents.sessionReady;
  VerificationThunksDetail::Emit(ProgressSink, MoveTemp(Ready));

  const TArray<FScenarioStep> ActiveSteps =
      ScenarioSelectors::SelectScenarioSteps(Store.getState().Scenario);
  VerificationThunksDetail::ProcessSteps(ActiveSteps, Runtime.numbers.emptyCount, Store,
                                ProgressSink);

  FQualityRunDependencies QualityDependencies;
  QualityDependencies.Host = qualityReportData().Host;
  QualityDependencies.Baseline = readQualityBaseline();
  QualityDependencies.ExecuteCommand =
      [&Store](const FCommandSpec &Command) {
        return VerificationThunksDetail::ExecuteCommand(Command, Store);
      };
  QualityDependencies.OnCommandCompleted =
      [&Store, &ProgressSink](
          const FCommandSpec &Command,
          const CommandRunner::FCommandOutput &CommandResult) {
        VerificationThunksDetail::CompleteQualityCommand(Command, CommandResult, Store,
                                                 ProgressSink);
      };
  runQualityEvaluation(Store, QualityDependencies);

  VerificationThunksDetail::ProcessFinalCommands(Data.finalCommands,
                                        Runtime.numbers.emptyCount, Store,
                                        ProgressSink);

  // The two-NPC chat is appended for the modes authored in twoNpcChatRunsInModes
  // (data-driven, mirrors TS runGame; autoplay alone must not run it).
  Runtime.twoNpcChatRunsInModes.Contains(Mode)
      ? (VerificationThunksDetail::AppendTwoNpcChat(Store), void())
      : void();

  FGameRunResult Result =
      VerificationSelectors::SelectGameRunResult(Store.getState());
  Result.QualityReport.hasValue
      ? (Result.QualityReportPath = displayQualityReportPath(
             writeCurrentQualityReport(Result.QualityReport.value)),
         void())
      : void();
  Result.Summary = VerificationSelectors::SelectGameSummaryText(Result);
  FGameProgress Completed;
  Completed.Type = Runtime.lifecycleEvents.sessionCompleted;
  Completed.RunResult = Result;
  VerificationThunksDetail::Emit(ProgressSink, MoveTemp(Completed));
  return Result;
  }();
}

} // namespace MicroGame
