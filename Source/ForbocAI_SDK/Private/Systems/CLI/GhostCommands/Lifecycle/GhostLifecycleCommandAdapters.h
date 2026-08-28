#pragma once

#include "Entities/CLI/Ghost/CLIGhostSelectors.h"
#include "Systems/CLI/Ghost/CLIGhostThunks.h"
#include "Systems/CLI/GhostCommands/Session/GhostSessionCommandAdapters.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace CLIOps::GhostCommands {

/** User Story: As Ghost lifecycle presentation, I need selected result lines emitted through one CLI effect boundary. @fn inline void LogGhostLines(const TArray<FString> &Lines) */
inline void LogGhostLines(const TArray<FString> &Lines) {
  func::for_each_array<FString>(
      Lines, [](const FString &Line) {
        ForbocAI::CLI::Presentation::logCliMessage(Line);
      });
}

/** User Story: As Ghost lifecycle startup, I need a suite and duration delegated to one Ghost SDK run operation. @fn inline GhostResult RunGhost(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First, int32 Second) */
inline GhostResult RunGhost(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const FString Suite =
      Args.Num() > State.EmptyCount ? Args[First] : State.DefaultSuite;
  const int32 Duration = Args.Num() > Second
                             ? FCString::Atoi(*Args[Second])
                             : State.DefaultDuration;
  ForbocAI::CLI::Presentation::logCliMessage(
      formatCliMessage(State.Starting, Suite));
  const FGhostRunResponse Response = Ops::startGhost(Store, Suite, Duration);
  ForbocAI::CLI::Presentation::logCliMessage(
      formatCliMessage(State.Started, Response.SessionId));
  return CLIOps::CommandResult::Success(Response.SessionId);
}

/** User Story: As Ghost lifecycle inspection, I need one explicit session delegated to the Ghost SDK status operation. @fn inline GhostResult ReadGhostStatus(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
inline GhostResult ReadGhostStatus(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  return WithRequiredSession(
      Args, State, First,
      [&](const FString &SessionId) {
        const FGhostStatus Status = Ops::getGhostStatus(Store, SessionId);
        LogGhostLines(
            ForbocAI::CLI::Ghost::selectCliGhostStatusLines(State, Status));
        return CLIOps::CommandResult::Success(Status.Status);
      });
}

/** User Story: As Ghost evidence inspection, I need one explicit session delegated to the Ghost SDK results operation. @fn inline GhostResult ReadGhostResults(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
inline GhostResult ReadGhostResults(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  return WithRequiredSession(
      Args, State, First,
      [&](const FString &SessionId) {
        const FGhostResults Results = Ops::getGhostResults(Store, SessionId);
        LogGhostLines(
            ForbocAI::CLI::Ghost::selectCliGhostResultsLines(State, Results));
        return CLIOps::CommandResult::Success(Results.SessionId);
      });
}

/** User Story: As Ghost lifecycle control, I need one explicit session delegated to the Ghost SDK stop operation. @fn inline GhostResult StopGhost(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
inline GhostResult StopGhost(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return WithRequiredSession(
      Args, State, First,
      [&](const FString &SessionId) {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Stopping, SessionId));
        const FGhostStopResponse Response = Ops::stopGhost(Store, SessionId);
        const FString Message =
            Response.bStopped ? State.Stopped : State.StopFailed;
        ForbocAI::CLI::Presentation::logCliMessage(Message);
        return Response.bStopped ? CLIOps::CommandResult::Success(Message)
                                 : CLIOps::CommandResult::Failure(Message);
      });
}

/** User Story: As Ghost history inspection, I need one bounded history request delegated to the Ghost SDK. @fn inline GhostResult ReadGhostHistory(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
inline GhostResult ReadGhostHistory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  const int32 Limit = Args.Num() > State.EmptyCount
                          ? FCString::Atoi(*Args[First])
                          : State.HistoryLimit;
  ForbocAI::CLI::Presentation::logCliMessage(State.HistoryTitle);
  LogGhostLines(ForbocAI::CLI::Ghost::selectCliGhostHistoryLines(
      State, Ops::getGhostHistory(Store, Limit)));
  return CLIOps::CommandResult::Success(State.HistoryTitle);
}

/** User Story: As Ghost lifecycle routing, I need session command roles composed as a focused immutable dispatcher. @fn inline func::Dispatcher<FString, GhostResult> CreateGhostLifecycleDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First, int32 Second) */
inline func::Dispatcher<FString, GhostResult> CreateGhostLifecycleDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First,
    int32 Second) {
  return func::createDispatcher<FString, GhostResult>({
      {Roles.GhostRun,
       [&]() { return RunGhost(Store, Args, State, First, Second); }},
      {Roles.GhostStatus,
       [&]() { return ReadGhostStatus(Store, Args, State, First); }},
      {Roles.GhostResults,
       [&]() { return ReadGhostResults(Store, Args, State, First); }},
      {Roles.GhostStop,
       [&]() { return StopGhost(Store, Args, State, First); }},
      {Roles.GhostHistory,
       [&]() { return ReadGhostHistory(Store, Args, State, First); }},
  });
}

} // namespace CLIOps::GhostCommands
