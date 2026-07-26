#pragma once

#include "HAL/PlatformProcess.h"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerActions.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerSelectors.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/Progress/ProgressThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame::GameThunksDetail {

/** User Story: As a command coverage runner, I need to reduce real CLI results through a stable signature so game state reflects each exercised SDK command. @fn inline void ApplyCommandResult( const FCommandSpec &Command, const CommandRunner::FCommandOutput &CommandResult, FTestGameStore &Store) */
inline void ApplyCommandResult(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &CommandResult,
    FTestGameStore &Store) {
  const FGameRuntimeData &Data = GameAdapters::GameRuntimeData();
  Command.Group == Data.commandGroups.npc_process_chat
      ? [&]() {
          const FParsedVerdict Verdict =
              GameAdapters::ParseVerdict(CommandResult.Output);
          const FString NpcId = GameAdapters::ExtractNpcId(Command.Command);
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
              GameAdapters::FormatGameTemplate(
                  Data.messages.bridgeValidationFailed, Values)));
        }()
      : void();
}

/** User Story: As a command runner, I need post-command pacing derived from authored cross-platform configuration. @fn inline int32 CommandDelayMs() */
inline int32 CommandDelayMs() {
  const FGameData &Data = GameAdapters::GameData();
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
          GameAdapters::GameData().numbers.millisecondsPerSecond);
  Seconds > static_cast<float>(
                GameAdapters::GameData().numbers.minimumDelayMs)
      ? FPlatformProcess::Sleep(Seconds)
      : void();
}

/** User Story: As a command runner, I need all aliases selected from the single package root store. @fn inline CommandRunner::FCommandAliasState SelectCommandAliases(const FTestGameStore &Store) */
inline CommandRunner::FCommandAliasState
SelectCommandAliases(const FTestGameStore &Store) {
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
 * @fn inline CommandRunner::FCommandOutput ExecuteCommand( const FCommandSpec &Command, FTestGameStore &Store)
 */
inline CommandRunner::FCommandOutput ExecuteCommand(
    const FCommandSpec &Command, FTestGameStore &Store) {
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

/** User Story: As a scenario runner, I need each authored SDK CLI result reduced through one root store and emitted for presentation. @fn inline void ProcessCommand(const FScenarioStep &Step, const FCommandSpec &Command, FTestGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommand(const FScenarioStep &Step,
                           const FCommandSpec &Command,
                           FTestGameStore &Store,
                           const FGameProgressSink &Sink) {
  const CommandRunner::FCommandOutput Result =
      ExecuteCommand(Command, Store);
  Result.Status == GameAdapters::GameRuntimeData().statuses.ok
      ? (Store.dispatch(CoverageActions::markCovered(Command.Group)), void())
      : void();
  TranscriptActions::FRecordTranscriptPayload Transcript;
  Transcript.ScenarioId = Step.Id;
  Transcript.CommandGroup = Command.Group;
  Transcript.Command = Command.Command;
  Transcript.ExpectedRoutes = Command.ExpectedRoutes;
  Transcript.Status = Result.Status;
  Transcript.Output = Result.Output;
  Transcript.DurationMs = Result.DurationMs;
  Store.dispatch(TranscriptActions::recordTranscript(Transcript));
  Result.Status == GameAdapters::GameRuntimeData().statuses.ok
      ? (ApplyCommandResult(Command, Result, Store), void())
      : void();
  FGameProgress Progress;
  Progress.Type =
      GameAdapters::GameRuntimeData().lifecycleEvents.commandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

/**
 * User Story: As a live model evaluator, I need quality command evidence recorded through the canonical game lifecycle.
 * @fn inline void CompleteQualityCommand( const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result, FTestGameStore &Store, const FGameProgressSink &Sink)
 */
inline void CompleteQualityCommand(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result, FTestGameStore &Store,
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
      GameAdapters::GameRuntimeData().lifecycleEvents.commandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

/** User Story: As a scenario runner, I need authored commands sequenced recursively through one SDK CLI boundary. @fn inline void ProcessCommands(const FScenarioStep &Step, int32 Index, FTestGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommands(const FScenarioStep &Step, int32 Index,
                            FTestGameStore &Store,
                            const FGameProgressSink &Sink) {
  Index >= Step.Commands.Num()
      ? void()
      : (ProcessCommand(Step, Step.Commands[Index], Store, Sink),
         DelayAfterCommand(),
         ProcessCommands(
             Step,
             Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
             Store, Sink));
}

} // namespace TestGame::GameThunksDetail
