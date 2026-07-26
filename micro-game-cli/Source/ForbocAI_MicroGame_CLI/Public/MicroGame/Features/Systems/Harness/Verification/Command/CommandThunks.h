#pragma once

#include "HAL/PlatformProcess.h"
#include "MicroGame/Features/Entities/NPCs/NPCsActions.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerActions.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerSelectors.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Progress/ProgressThunks.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"
#include "MicroGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"
#include "MicroGame/Features/Systems/Terminal/UI/UIActions.h"

namespace MicroGame::VerificationThunksDetail {

/** User Story: As a command coverage runner, I need to reduce real CLI results through a stable signature so game state reflects each exercised SDK command. @fn inline void ApplyCommandResult( const FCommandSpec &Command, const CommandRunner::FCommandOutput &CommandResult, FMicroGameStore &Store) */
inline void ApplyCommandResult(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &CommandResult,
    FMicroGameStore &Store) {
  const FGameRuntimeData &Data = VerificationVocabularyAdapters::GameRuntimeData();
  Command.Group == Data.commandGroups.npc_process_chat
      ? [&]() {
          const FParsedVerdict Verdict =
              VerificationAdapters::ParseVerdict(CommandResult.Output);
          const FString NpcId = VerificationAdapters::ExtractNpcId(Command.Command);
          (Verdict.bValid && !NpcId.IsEmpty())
              ? [&]() {
                  NPCsActions::FApplyNpcVerdictPayload Payload;
                  Payload.Id = NpcId;
                  Payload.Action.Type = Verdict.ActionType;
                  Payload.Action.TargetHex = Verdict.TargetHex;
                  Payload.Action.bHasTargetHex = true;
                  Payload.StateDelta.Suspicion = Verdict.SuspicionDelta;
                  Payload.StateDelta.bHasSuspicion =
                      Verdict.SuspicionDelta != int32{};
                  Store.dispatch(NPCsActions::ApplyNpcVerdict(Payload));
                }()
              : void();
        }()
      : void();

  (Command.Group == Data.commandGroups.bridge_validate &&
   CommandResult.Status == Data.statuses.error)
      ? [&]() {
          TMap<FString, FString> Values;
          Values.Add(Data.tokens.output, CommandResult.Output);
          Store.dispatch(UIActions::addMessage(
              VerificationAdapters::FormatGameTemplate(
                  Data.messages.bridgeValidationFailed, Values)));
        }()
      : void();
}

/** User Story: As a command runner, I need post-command pacing derived from authored cross-platform configuration. @fn inline int32 CommandDelayMs() */
inline int32 CommandDelayMs() {
  const FGameData &Data = VerificationAdapters::GameData();
  const FString Raw = FPlatformMisc::GetEnvironmentVariable(
      *Data.environment.commandDelayKey);
  const int32 Parsed = Raw.IsEmpty()
                           ? Data.environment.defaultCommandDelayMs
                           : FCString::Atoi(*Raw);
  return Parsed >= Data.numbers.minimumDelayMs
             ? Parsed
             : Data.environment.defaultCommandDelayMs;
}

/** User Story: As a command runner, I need positive authored pacing applied at the process effect boundary. @fn inline void DelayAfterCommand() */
inline void DelayAfterCommand() {
  const float Seconds =
      static_cast<float>(CommandDelayMs()) /
      static_cast<float>(
          VerificationAdapters::GameData().numbers.millisecondsPerSecond);
  Seconds > static_cast<float>(
                VerificationAdapters::GameData().numbers.minimumDelayMs)
      ? FPlatformProcess::Sleep(Seconds)
      : void();
}

/** User Story: As a command runner, I need all aliases selected from the single package root store. @fn inline CommandRunner::FCommandAliasState SelectCommandAliases(const FMicroGameStore &Store) */
inline CommandRunner::FCommandAliasState
SelectCommandAliases(const FMicroGameStore &Store) {
  return CommandRunner::CreateCommandAliasState(
      CommandRunnerSelectors::SelectNpcAliases(
          Store.getState().CommandRunner),
      CommandRunnerSelectors::SelectGhostSessionAliases(
          Store.getState().CommandRunner),
      CommandRunnerSelectors::SelectSoulTransactionAliases(
          Store.getState().CommandRunner));
}

/**
 * User Story: As a game and quality harness, I need all command families assertion-validated through one public CLI boundary.
 * @fn inline CommandRunner::FCommandOutput ExecuteCommand( const FCommandSpec &Command, FMicroGameStore &Store)
 */
inline CommandRunner::FCommandOutput ExecuteCommand(
    const FCommandSpec &Command, FMicroGameStore &Store) {
  const CommandRunner::FCommandOutput ExecutionResult =
      CommandRunner::Execute(Command.Command, SelectCommandAliases(Store));
  CommandRunner::HasCommandAliasUpdate(ExecutionResult.AliasUpdate)
      ? (Store.dispatch(CommandRunnerActions::aliasesCaptured(
             ExecutionResult.AliasUpdate)),
         void())
      : void();
  return CommandRunner::ValidateOutputAssertions(
      Command, ExecutionResult, SelectCommandAliases(Store));
}

/**
 * User Story: As a scenario runner, I need each actual SDK CLI result reduced through one root store and emitted without equating coverage with success.
 * @fn inline void RecordCommandResult( const FScenarioStep &Step, const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result, FMicroGameStore &Store, const FGameProgressSink &Sink)
 */
inline void RecordCommandResult(
    const FScenarioStep &Step, const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result, FMicroGameStore &Store,
    const FGameProgressSink &Sink) {
  Store.dispatch(CoverageActions::markCovered(Command.Group));
  TranscriptActions::FRecordTranscriptPayload Transcript;
  Transcript.ScenarioId = Step.Id;
  Transcript.CommandGroup = Command.Group;
  Transcript.Command = Command.Command;
  Transcript.ExpectedRoutes = Command.ExpectedRoutes;
  Transcript.Status = Result.Status;
  Transcript.Output = Result.Output;
  Transcript.DurationMs = Result.DurationMs;
  Store.dispatch(TranscriptActions::recordTranscript(Transcript));
  Result.Status == VerificationVocabularyAdapters::GameRuntimeData().statuses.ok
      ? (ApplyCommandResult(Command, Result, Store), void())
      : void();
  FGameProgress Progress;
  Progress.Type =
      VerificationVocabularyAdapters::GameRuntimeData().lifecycleEvents.commandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

/** User Story: As a scenario runner, I need each authored command executed and recorded through one shared SDK CLI boundary. @fn inline void ProcessCommand(const FScenarioStep &Step, const FCommandSpec &Command, FMicroGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommand(const FScenarioStep &Step,
                           const FCommandSpec &Command,
                           FMicroGameStore &Store,
                           const FGameProgressSink &Sink) {
  RecordCommandResult(Step, Command, ExecuteCommand(Command, Store), Store,
                      Sink);
}

/**
 * User Story: As a live model evaluator, I need quality command evidence recorded through the canonical game lifecycle.
 * @fn inline void CompleteQualityCommand( const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result, FMicroGameStore &Store, const FGameProgressSink &Sink)
 */
inline void CompleteQualityCommand(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result, FMicroGameStore &Store,
    const FGameProgressSink &Sink) {
  TranscriptActions::FRecordTranscriptPayload Transcript;
  Transcript.ScenarioId = Command.Group;
  Transcript.CommandGroup = Command.Group;
  Transcript.Command = Command.Command;
  Transcript.ExpectedRoutes = Command.ExpectedRoutes;
  Transcript.Status = Result.Status;
  Transcript.Output = Result.Output;
  Transcript.DurationMs = Result.DurationMs;
  Store.dispatch(TranscriptActions::recordTranscript(Transcript));
  FGameProgress Progress;
  Progress.Type =
      VerificationVocabularyAdapters::GameRuntimeData().lifecycleEvents.commandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

/** User Story: As a scenario runner, I need authored commands sequenced recursively through one SDK CLI boundary. @fn inline void ProcessCommands(const FScenarioStep &Step, int32 Index, FMicroGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommands(const FScenarioStep &Step, int32 Index,
                            FMicroGameStore &Store,
                            const FGameProgressSink &Sink) {
  Index >= Step.Commands.Num()
      ? void()
      : (ProcessCommand(Step, Step.Commands[Index], Store, Sink),
         DelayAfterCommand(),
         ProcessCommands(
             Step,
             Index + VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex,
             Store, Sink));
}

/** User Story: As a complete-game verifier, I need authored final CLI checks executed once when the API scenario did not already cover them. @fn inline void ProcessFinalCommands(const TArray<FCommandSpec> &Commands, int32 Index, FMicroGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessFinalCommands(const TArray<FCommandSpec> &Commands,
                                 int32 Index, FMicroGameStore &Store,
                                 const FGameProgressSink &Sink) {
  Index >= Commands.Num()
      ? void()
      : [&]() {
          const FCommandSpec &Command = Commands[Index];
          const bool bCovered = func::map_value_or<FString, bool>(
              CoverageSelectors::SelectHarnessCovered(
                  Store.getState().Harness),
              Command.Group, false);
          !bCovered
              ? [&]() {
                  FScenarioStep Step;
                  Step.Id = Command.Group;
                  ProcessCommand(Step, Command, Store, Sink);
                  DelayAfterCommand();
                }()
              : void();
          ProcessFinalCommands(
              Commands,
              Index + VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex,
              Store, Sink);
        }();
}

} // namespace MicroGame::VerificationThunksDetail
