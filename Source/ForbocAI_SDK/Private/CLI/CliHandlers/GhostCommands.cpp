#include "CLI/CliHandlers.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/Ghost/CLIGhostSelectors.h"
#include "Features/CLI/Ghost/CLIGhostThunks.h"
#include "Features/CLI/Presentation/PresentationAdapters.h"
#include "Store.h"

namespace {

/** User Story: As a Ghost CLI consumer, I need semantic success output converted for the command runner. @fn CLIOps::Handlers::Result GhostSuccess(const FString &Message) */
CLIOps::Handlers::Result GhostSuccess(const FString &Message) {
  return CLIOps::Handlers::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a Ghost CLI consumer, I need semantic failure output converted for the command runner. @fn CLIOps::Handlers::Result GhostFailure(const FString &Message) */
CLIOps::Handlers::Result GhostFailure(const FString &Message) {
  return CLIOps::Handlers::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a Ghost CLI presenter, I need selected lines emitted through one effect boundary. @fn void LogGhostLines(const TArray<FString> &Lines) */
void LogGhostLines(const TArray<FString> &Lines) {
  func::for_each_array<FString>(
      Lines, [](const FString &Line) {
        ForbocAI::CLI::Presentation::logCliMessage(Line);
      });
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a CLI user, I need Ghost commands routed through root-store state, thunks, and selectors so the command edge remains thin. @fn HandlerResult HandleGhost(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleGhost(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  using ForbocAI::CLI::Presentation::formatCliMessage;
  using ForbocAI::CLI::Presentation::logCliMessage;
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Ghost::FCLIGhostState &State =
      ForbocAI::CLI::Ghost::selectCliGhost(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;

  return CommandKey == Roles.GhostRun
             ? [&]() -> HandlerResult {
                 const FString Suite =
                     Args.Num() > State.EmptyCount
                         ? Args[First]
                         : State.DefaultSuite;
                 const int32 Duration =
                     Args.Num() > Second ? FCString::Atoi(*Args[Second])
                                         : State.DefaultDuration;
                 logCliMessage(
                     formatCliMessage(State.Starting, Suite));
                 const FGhostRunResponse Response =
                     Ops::startGhost(Store, Suite, Duration);
                 logCliMessage(formatCliMessage(State.Started,
                                                Response.SessionId));
                 return just(GhostSuccess(Response.SessionId));
               }()
         : CommandKey == Roles.GhostStatus
             ? (Args.Num() <= First
                    ? just(GhostFailure(State.RequiredSession))
                    : [&]() -> HandlerResult {
                        const FGhostStatus Status =
                            Ops::getGhostStatus(Store, Args[First]);
                        LogGhostLines(
                            ForbocAI::CLI::Ghost::selectCliGhostStatusLines(
                                State, Status));
                        return just(GhostSuccess(Status.Status));
                      }())
         : CommandKey == Roles.GhostResults
             ? (Args.Num() <= First
                    ? just(GhostFailure(State.RequiredSession))
                    : [&]() -> HandlerResult {
                        const FGhostResults Results =
                            Ops::getGhostResults(Store, Args[First]);
                        LogGhostLines(
                            ForbocAI::CLI::Ghost::selectCliGhostResultsLines(
                                State, Results));
                        return just(GhostSuccess(Results.SessionId));
                      }())
         : CommandKey == Roles.GhostStop
             ? (Args.Num() <= First
                    ? just(GhostFailure(State.RequiredSession))
                    : [&]() -> HandlerResult {
                        logCliMessage(formatCliMessage(
                            State.Stopping, Args[First]));
                        const FGhostStopResponse Response =
                            Ops::stopGhost(Store, Args[First]);
                        const FString Message =
                            Response.bStopped ? State.Stopped
                                              : State.StopFailed;
                        logCliMessage(Message);
                        return just(Response.bStopped
                                        ? GhostSuccess(Message)
                                        : GhostFailure(Message));
                      }())
         : CommandKey == Roles.GhostHistory
             ? [&]() -> HandlerResult {
                 const int32 Limit =
                     Args.Num() > State.EmptyCount
                         ? FCString::Atoi(*Args[First])
                         : State.HistoryLimit;
                 logCliMessage(State.HistoryTitle);
                 const TArray<FGhostHistoryEntry> History =
                     Ops::getGhostHistory(Store, Limit);
                 LogGhostLines(
                     ForbocAI::CLI::Ghost::selectCliGhostHistoryLines(
                         State, History));
                 return just(GhostSuccess(State.HistoryTitle));
               }()
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
