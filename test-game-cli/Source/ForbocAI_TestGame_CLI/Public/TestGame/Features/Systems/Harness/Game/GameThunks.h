#pragma once

#include "HAL/PlatformProcess.h"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Systems/Contract/ContractThunks.h"
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
  const FString Raw = FPlatformMisc::GetEnvironmentVariable(
      TEXT("FORBOCAI_TEST_GAME_COMMAND_DELAY_MS"));
  const int32 Parsed = Raw.IsEmpty() ? 1000 : FCString::Atoi(*Raw);
  return Parsed >= 0 ? Parsed : 1000;
}

/** User Story: As a systems harness game consumer, I need to invoke delay after command through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void DelayAfterCommand() */
inline void DelayAfterCommand() {
  const float Seconds = static_cast<float>(CommandDelayMs()) / 1000.0f;
  Seconds > 0.0f ? FPlatformProcess::Sleep(Seconds) : void();
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

/** User Story: As a systems harness game consumer, I need to invoke process command through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void ProcessCommand(const FScenarioStep &Step, const FCommandSpec &Command, FTestGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommand(const FScenarioStep &Step,
                           const FCommandSpec &Command,
                           FTestGameStore &Store,
                           const FGameProgressSink &Sink) {
  const CommandRunner::FCommandOutput ExecutionResult =
      CommandRunner::Execute(Command.Command, SelectCommandAliases(Store));
  CommandRunner::HasCommandAliasUpdate(ExecutionResult.AliasUpdate)
      ? (Store.dispatch(CommandRunnerActions::aliasesCaptured(
             ExecutionResult.AliasUpdate)),
         void())
      : void();
  const CommandRunner::FCommandOutput Result =
      CommandRunner::ValidateOutputAssertions(
          Command, ExecutionResult, SelectCommandAliases(Store));
  Result.Status == ETranscriptStatus::Ok
      ? (Store.dispatch(HarnessActions::markCovered(Command.Group)), void())
      : void();

  TranscriptActions::FRecordTranscriptPayload Transcript;
  Transcript.ScenarioId = Step.Id;
  Transcript.CommandGroup = Command.Group;
  Transcript.Command = Command.Command;
  Transcript.ExpectedRoutes = Command.ExpectedRoutes;
  Transcript.Status = Result.Status;
  Transcript.Output = Result.Output;
  Store.dispatch(TranscriptActions::recordTranscript(Transcript));

  Result.Status == ETranscriptStatus::Ok
      ? (ApplyCommandResult(Command, Result, Store), void())
      : void();

  FGameProgress Progress;
  Progress.Type = EGameProgressType::CommandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

/** User Story: As a systems harness game consumer, I need to invoke process commands through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void ProcessCommands(const FScenarioStep &Step, int32 Index, FTestGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessCommands(const FScenarioStep &Step, int32 Index,
                            FTestGameStore &Store,
                            const FGameProgressSink &Sink) {
  Index >= Step.Commands.Num()
      ? void()
      : (ProcessCommand(Step, Step.Commands[Index], Store, Sink),
         DelayAfterCommand(),
         ProcessCommands(Step, Index + 1, Store, Sink));
}

/** User Story: As a systems harness game consumer, I need to invoke process steps through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index, FTestGameStore &Store, const FGameProgressSink &Sink) */
inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index,
                         FTestGameStore &Store,
                         const FGameProgressSink &Sink) {
  Index >= Steps.Num()
      ? void()
      : [&]() {
          FGameProgress Progress;
          Progress.Type = EGameProgressType::StepStarted;
          Progress.Step = Steps[Index];
          Emit(Sink, MoveTemp(Progress));
          ApplyScenarioInitialState(Steps[Index], Store);
          ProcessCommands(Steps[Index], 0, Store, Sink);
          ProcessSteps(Steps, Index + 1, Store, Sink);
        }();
}

/** User Story: As a systems harness game consumer, I need to invoke fail run through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FGameRunResult FailRun(const FString &Message, const FGameProgressSink &Sink) */
inline FGameRunResult FailRun(const FString &Message,
                              const FGameProgressSink &Sink) {
  FGameRunResult Failure;
  Failure.Summary = Message;
  FGameProgress Progress;
  Progress.Type = EGameProgressType::ContractFailed;
  Progress.Message = Message;
  Emit(Sink, MoveTemp(Progress));
  return Failure;
}

} // namespace GameThunksDetail

/** User Story: As a systems harness game consumer, I need to invoke run game through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FGameRunResult RunGame(FTestGameStore &Store, EPlayMode Mode, const FString &ApiUrlOverride, const FGameProgressSink &ProgressSink = {}) */
inline FGameRunResult RunGame(FTestGameStore &Store, EPlayMode Mode,
                              const FString &ApiUrlOverride,
                              const FGameProgressSink &ProgressSink = {}) {
  SDKConfig::InitializeConfig();
  Store.dispatch(CommandRunnerActions::aliasesReset());
  Store.dispatch(UIActions::setMode(Mode));

  FGameProgress Started;
  Started.Type = EGameProgressType::SessionStarted;
  Started.Mode = Mode;
  GameThunksDetail::Emit(ProgressSink, MoveTemp(Started));

  FGameNPC PlayerNpc;
  PlayerNpc.Id = TEXT("scout");
  PlayerNpc.Name = TEXT("Scout");
  PlayerNpc.Faction = TEXT("Player");
  PlayerNpc.Hp = 100;
  PlayerNpc.Suspicion = 0;
  PlayerNpc.Inventory.Add(TEXT("coin-pouch"));
  PlayerNpc.Position = FPosition(1, 1);
  Store.dispatch(NPCsActions::UpsertNPC(PlayerNpc));

  const FString ApiUrl = !ApiUrlOverride.IsEmpty()
                             ? ApiUrlOverride
                             : ResolveRuntimeUrl();
  return ApiUrl.IsEmpty()
             ? GameThunksDetail::FailRun(TEXT("Runtime URL not configured"),
                                         ProgressSink)
             : [&]() {
                 const Contract::FContractResponse Contract =
                     Contract::GetContract(Store, ApiUrl);
                 return !Contract.bValid
                            ? GameThunksDetail::FailRun(
                                  TEXT("API contract unavailable"),
                                  ProgressSink)
                            : [&]() {
                                Store.dispatch(
                                    ScenarioActions::contractReceived(
                                        Contract::ToScenarioSteps(
                                            Contract.Scenarios, 0, {})));
                                const TArray<FScenarioStep> Steps =
                                    ScenarioSelectors::SelectScenarioSteps(
                                        Store.getState().Scenario);
                                GameThunksDetail::ProcessSteps(
                                    Steps, 0, Store, ProgressSink);

                                FGameRunResult Result =
                                    GameSelectors::SelectGameRunResult(
                                        Store.getState(),
                                        Contract::ToCommandGroups(
                                            Contract.RequiredCommandGroups, 0,
                                            {}));
                                FGameProgress Completed;
                                Completed.Type =
                                    EGameProgressType::SessionCompleted;
                                Completed.RunResult = Result;
                                GameThunksDetail::Emit(
                                    ProgressSink, MoveTemp(Completed));
                                return Result;
                              }();
               }();
}

} // namespace TestGame
