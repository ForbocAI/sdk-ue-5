#pragma once

#include "Features/Config/ConfigActions.h"
#include "Features/Config/ConfigSelectors.h"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Systems/Contract/Parsing/ContractParsingAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/Command/CommandThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/Progress/ProgressThunks.h"
#include "TestGame/Features/Systems/Harness/Game/Scenario/ScenarioThunks.h"
#include "TestGame/Features/Systems/Harness/HarnessThunks.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "TestGame/Features/Systems/Quality/QualityThunks.h"
#include "TestGame/Features/Systems/Quality/Report/ReportAdapters.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

namespace GameThunksDetail {

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
  !ApiUrlOverride.IsEmpty()
      ? (Store.dispatch(ConfigSlice::Actions::apiConfigurationCommitted(
             {ApiUrlOverride,
              ConfigSelectors::selectApiKey(Store.getState())})),
         void())
      : void();
  Store.dispatch(CommandRunnerActions::aliasesReset());
  Store.dispatch(UIActions::setMode(Mode));

  FGameProgress Started;
  Started.Type =
      GameAdapters::GameRuntimeData().lifecycleEvents.sessionStarted;
  Started.Mode = Mode;
  GameThunksDetail::Emit(ProgressSink, MoveTemp(Started));

  Store.dispatch(NPCsActions::UpsertNPC(
      GameAdapters::GameRuntimeData().initialState.sessionNpc));

  const FString ApiUrl = ConfigSelectors::selectApiUrl(Store.getState());
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

                                FQualityRunDependencies QualityDependencies;
                                QualityDependencies.Host =
                                    qualityReportData().Host;
                                QualityDependencies.Baseline =
                                    readQualityBaseline();
                                QualityDependencies.ExecuteCommand =
                                    [&Store, &ApiUrl](
                                        const FCommandSpec &Command) {
                                      return GameThunksDetail::ExecuteCommand(
                                          Command, Store, ApiUrl);
                                    };
                                QualityDependencies.OnCommandCompleted =
                                    [&Store, &ProgressSink](
                                        const FCommandSpec &Command,
                                        const CommandRunner::FCommandOutput
                                            &CommandResult) {
                                      GameThunksDetail::CompleteQualityCommand(
                                          Command, CommandResult, Store,
                                          ProgressSink);
                                    };
                                runQualityEvaluation(Store,
                                                     QualityDependencies);

                                FGameRunResult Result =
                                    GameSelectors::SelectGameRunResult(
                                        Store.getState());
                                Result.QualityReport.hasValue
                                    ? (Result.QualityReportPath =
                                           displayQualityReportPath(
                                               writeCurrentQualityReport(
                                                   Result.QualityReport.value)),
                                       void())
                                    : void();
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
