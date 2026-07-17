#pragma once

#include "HAL/PlatformProcess.h"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Systems/Contract/Parsing/ContractParsingAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerActions.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerSelectors.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Game/Command/CommandThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/Scenario/ScenarioThunks.h"
#include "TestGame/Features/Systems/Harness/HarnessThunks.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

using FGameProgressSink = TFunction<void(const FGameProgress &)>;

namespace GameThunksDetail {

/** User Story: As a systems harness game consumer, I need to invoke emit through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) */
inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) {
  Sink ? (Sink(Progress), void()) : void();
}

/** User Story: As a systems harness game consumer, I need to invoke command delay ms through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline int32 CommandDelayMs() */
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

/** User Story: As a systems harness game consumer, I need to invoke delay after command through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void DelayAfterCommand() */
inline void DelayAfterCommand() {
  const float Seconds =
      static_cast<float>(CommandDelayMs()) /
      static_cast<float>(
          GameAdapters::GameData().numbers.millisecondsPerSecond);
  Seconds > static_cast<float>(GameAdapters::GameData()
                                   .numbers.minimumDelayMs)
      ? FPlatformProcess::Sleep(Seconds)
      : void();
}

/** User Story: As a systems harness game consumer, I need the complete command alias state selected from the package root store through a stable signature so execution and semantic validation share one identity source. @fn inline CommandRunner::FCommandAliasState SelectCommandAliases(const FTestGameStore &Store) */
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

/** User Story: As a systems harness game consumer, I need each authored command executed against the selected API through one root store before presentation observes progress. @fn inline void ProcessCommand(const FScenarioStep &Step, const FCommandSpec &Command, FTestGameStore &Store, const FString &ApiUrl, const FGameProgressSink &Sink) */
inline void ProcessCommand(const FScenarioStep &Step,
                           const FCommandSpec &Command,
                           FTestGameStore &Store,
                           const FString &ApiUrl,
                           const FGameProgressSink &Sink) {
  const CommandRunner::FCommandOutput ExecutionResult =
      CommandRunner::Execute(Command.Command, Store, ApiUrl,
                             SelectCommandAliases(Store));
  CommandRunner::HasCommandAliasUpdate(ExecutionResult.AliasUpdate)
      ? (Store.dispatch(CommandRunnerActions::aliasesCaptured(
             ExecutionResult.AliasUpdate)),
         void())
      : void();
  const CommandRunner::FCommandOutput Result =
      CommandRunner::ValidateOutputAssertions(
          Command, ExecutionResult, SelectCommandAliases(Store));
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

/** User Story: As a systems harness game consumer, I need a scenario's authored commands sequenced against one API and root store so coverage cannot bypass public CLI execution. @fn inline void ProcessCommands(const FScenarioStep &Step, int32 Index, FTestGameStore &Store, const FString &ApiUrl, const FGameProgressSink &Sink) */
inline void ProcessCommands(const FScenarioStep &Step, int32 Index,
                            FTestGameStore &Store,
                            const FString &ApiUrl,
                            const FGameProgressSink &Sink) {
  Index >= Step.Commands.Num()
      ? void()
      : (ProcessCommand(Step, Step.Commands[Index], Store, ApiUrl, Sink),
         DelayAfterCommand(),
         ProcessCommands(
             Step,
             Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
             Store, ApiUrl, Sink));
}

/** User Story: As a systems harness game consumer, I need contract-authored scenarios sequenced against one API and root store so the complete CLI contract is exercised. @fn inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index, FTestGameStore &Store, const FString &ApiUrl, const FGameProgressSink &Sink) */
inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index,
                         FTestGameStore &Store,
                         const FString &ApiUrl,
                         const FGameProgressSink &Sink) {
  Index >= Steps.Num()
      ? void()
      : [&]() {
          FGameProgress Progress;
          Progress.Type =
              GameAdapters::GameRuntimeData().lifecycleEvents.stepStarted;
          Progress.Step = Steps[Index];
          Emit(Sink, MoveTemp(Progress));
          ApplyScenarioInitialState(Steps[Index], Store);
          ProcessCommands(
              Steps[Index],
              GameAdapters::GameRuntimeData().numbers.emptyCount, Store,
              ApiUrl, Sink);
          ProcessSteps(
              Steps,
              Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
              Store, ApiUrl, Sink);
        }();
}

/** User Story: As a systems harness game consumer, I need to invoke fail run through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FGameRunResult FailRun(const FString &Message, const FGameProgressSink &Sink) */
inline FGameRunResult FailRun(const FString &Message,
                              const FGameProgressSink &Sink) {
  FGameRunResult Failure;
  Failure.Summary = Message;
  FGameProgress Progress;
  Progress.Type =
      GameAdapters::GameRuntimeData().lifecycleEvents.contractFailed;
  Progress.Message = Message;
  Emit(Sink, MoveTemp(Progress));
  return Failure;
}

} // namespace GameThunksDetail

/** User Story: As a systems harness game consumer, I need to invoke run game through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FGameRunResult RunGame(FTestGameStore &Store, FString Mode, const FString &ApiUrlOverride, const FGameProgressSink &ProgressSink = {}) */
inline FGameRunResult RunGame(FTestGameStore &Store, FString Mode,
                              const FString &ApiUrlOverride,
                              const FGameProgressSink &ProgressSink = {}) {
  SDKConfig::InitializeConfig();
  Store.dispatch(CommandRunnerActions::aliasesReset());
  Store.dispatch(UIActions::setMode(Mode));

  FGameProgress Started;
  Started.Type =
      GameAdapters::GameRuntimeData().lifecycleEvents.sessionStarted;
  Started.Mode = Mode;
  GameThunksDetail::Emit(ProgressSink, MoveTemp(Started));

  Store.dispatch(NPCsActions::UpsertNPC(
      GameAdapters::GameRuntimeData().initialState.sessionNpc));

  const FString ApiUrl = !ApiUrlOverride.IsEmpty()
                             ? ApiUrlOverride
                             : ResolveRuntimeUrl();
  return ApiUrl.IsEmpty()
             ? GameThunksDetail::FailRun(
                   GameAdapters::GameData().messages.runtimeUrlMissing,
                   ProgressSink)
             : [&]() {
                 const CommandRunner::FCommandOutput ContractResult =
                     CommandRunner::Execute(
                         GameAdapters::GameData().contractCommand.Command,
                         Store, ApiUrl,
                         GameThunksDetail::SelectCommandAliases(Store));
                 const Contract::FContractResponse Contract =
                     ContractResult.Status ==
                             GameAdapters::GameRuntimeData().statuses.ok
                         ? Contract::ParseContractJson(ContractResult.Output)
                         : Contract::FContractResponse{};
                 return !Contract.bValid
                            ? GameThunksDetail::FailRun(
                                  GameAdapters::GameData()
                                      .messages.apiContractUnavailable,
                                  ProgressSink)
                            : [&]() {
                                FScenarioContractPayload ContractPayload;
                                ContractPayload.RequiredCommandGroups =
                                    Contract.RequiredCommandGroups;
                                ContractPayload.Steps = Contract.Scenarios;
                                Store.dispatch(ScenarioActions::setContract(
                                    MoveTemp(ContractPayload)));
                                const TArray<FScenarioStep> Steps =
                                    ScenarioSelectors::SelectScenarioSteps(
                                        Store.getState().Scenario);
                                GameThunksDetail::ProcessSteps(
                                    Steps,
                                    GameAdapters::GameRuntimeData()
                                        .numbers.emptyCount,
                                    Store, ApiUrl, ProgressSink);

                                FGameRunResult Result =
                                    GameSelectors::SelectGameRunResult(
                                        Store.getState());
                                Result.Summary =
                                    GameSelectors::SelectGameSummaryText(
                                        Result);
                                FGameProgress Completed;
                                Completed.Type =
                                    GameAdapters::GameRuntimeData()
                                        .lifecycleEvents.sessionCompleted;
                                Completed.RunResult = Result;
                                GameThunksDetail::Emit(
                                    ProgressSink, MoveTemp(Completed));
                                return Result;
                              }();
               }();
}

} // namespace TestGame
