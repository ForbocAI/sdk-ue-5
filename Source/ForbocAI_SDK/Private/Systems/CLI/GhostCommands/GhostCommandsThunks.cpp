#include "Systems/CLI/GhostCommands/GhostCommandsThunks.h"
#include "Entities/CLI/CLISelectors.h"
#include "Entities/CLI/Ghost/CLIGhostSelectors.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/Ghost/CLIGhostThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace {

using GhostResult = CLIOps::CommandRouting::Result;

/** User Story: As a Ghost CLI consumer, I need semantic success output converted for the command runner. @fn CLIOps::CommandRouting::Result GhostSuccess(const FString &Message) */
CLIOps::CommandRouting::Result GhostSuccess(const FString &Message) {
  return CLIOps::CommandRouting::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a Ghost CLI consumer, I need semantic failure output converted for the command runner. @fn CLIOps::CommandRouting::Result GhostFailure(const FString &Message) */
CLIOps::CommandRouting::Result GhostFailure(const FString &Message) {
  return CLIOps::CommandRouting::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a Ghost CLI presenter, I need selected lines emitted through one effect boundary. @fn void LogGhostLines(const TArray<FString> &Lines) */
void LogGhostLines(const TArray<FString> &Lines) {
  func::for_each_array<FString>(
      Lines, [](const FString &Line) {
        ForbocAI::CLI::Presentation::logCliMessage(Line);
      });
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke run ghost through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn GhostResult RunGhost( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First, int32 Second) */
GhostResult RunGhost(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const FString Suite =
      Args.Num() > State.EmptyCount ? Args[First] : State.DefaultSuite;
  const int32 Duration = Args.Num() > Second ? FCString::Atoi(*Args[Second])
                                             : State.DefaultDuration;
  ForbocAI::CLI::Presentation::logCliMessage(
      formatCliMessage(State.Starting, Suite));
  const FGhostRunResponse Response =
      Ops::startGhost(Store, Suite, Duration);
  ForbocAI::CLI::Presentation::logCliMessage(
      formatCliMessage(State.Started, Response.SessionId));
  return GhostSuccess(Response.SessionId);
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke read ghost status through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn GhostResult ReadGhostStatus( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
GhostResult ReadGhostStatus(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  return CLIOps::CommandRouting::matchCondition<GhostResult>(
      Args.Num() > First,
      [&]() {
        const FGhostStatus Status = Ops::getGhostStatus(Store, Args[First]);
        LogGhostLines(
            ForbocAI::CLI::Ghost::selectCliGhostStatusLines(State, Status));
        return GhostSuccess(Status.Status);
      },
      [&]() { return GhostFailure(State.RequiredSession); });
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke read ghost results through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn GhostResult ReadGhostResults( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
GhostResult ReadGhostResults(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  return CLIOps::CommandRouting::matchCondition<GhostResult>(
      Args.Num() > First,
      [&]() {
        const FGhostResults Results = Ops::getGhostResults(Store, Args[First]);
        LogGhostLines(
            ForbocAI::CLI::Ghost::selectCliGhostResultsLines(State, Results));
        return GhostSuccess(Results.SessionId);
      },
      [&]() { return GhostFailure(State.RequiredSession); });
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke stop ghost through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn GhostResult StopGhost( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
GhostResult StopGhost(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<GhostResult>(
      Args.Num() > First,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Stopping, Args[First]));
        const FGhostStopResponse Response = Ops::stopGhost(Store, Args[First]);
        const FString Message =
            Response.bStopped ? State.Stopped : State.StopFailed;
        ForbocAI::CLI::Presentation::logCliMessage(Message);
        return Response.bStopped ? GhostSuccess(Message)
                                 : GhostFailure(Message);
      },
      [&]() { return GhostFailure(State.RequiredSession); });
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke read ghost history through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn GhostResult ReadGhostHistory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) */
GhostResult ReadGhostHistory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First) {
  const int32 Limit = Args.Num() > State.EmptyCount
                          ? FCString::Atoi(*Args[First])
                          : State.HistoryLimit;
  ForbocAI::CLI::Presentation::logCliMessage(State.HistoryTitle);
  const TArray<FGhostHistoryEntry> History =
      Ops::getGhostHistory(Store, Limit);
  LogGhostLines(
      ForbocAI::CLI::Ghost::selectCliGhostHistoryLines(State, History));
  return GhostSuccess(State.HistoryTitle);
}

/** User Story: As a CLI command-routing ghost consumer, I need to invoke create ghost dispatcher through a stable signature so the CLI command-routing ghost workflow remains explicit and composable. @fn func::Dispatcher<FString, GhostResult> CreateGhostDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First, int32 Second) */
func::Dispatcher<FString, GhostResult>
CreateGhostDispatcher(rtk::EnhancedStore<FRuntimeState> &Store,
                      const ForbocAI::CLI::FCLICommandRoles &Roles,
                      const TArray<FString> &Args,
                      const ForbocAI::CLI::Ghost::FCLIGhostState &State,
                      int32 First, int32 Second) {
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

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI user, I need Ghost commands routed through root-store state, thunks, and selectors so the command edge remains thin. @fn RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Ghost::FCLIGhostState &State =
      ForbocAI::CLI::Ghost::selectCliGhost(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;
  return func::dispatch(
      CreateGhostDispatcher(Store, Roles, Args, State, First, Second),
      CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
