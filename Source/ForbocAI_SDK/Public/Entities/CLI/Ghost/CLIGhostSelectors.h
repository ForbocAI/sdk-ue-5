#pragma once

#include "Core/fp.hpp"
#include "Components/CLI/CLITypes.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Components/Ghost/GhostTypes.h"

namespace ForbocAI::CLI::Ghost {

/** User Story: As a Ghost CLI consumer, I need feature state selected from the package CLI slice. @fn inline const FCLIGhostState &selectCliGhost(const FCLIState &State) */
inline const FCLIGhostState &selectCliGhost(const FCLIState &State) {
  return State.Ghost;
}

/** User Story: As a QA operator, I need Ghost test results derived from typed SDK output. @fn inline TArray<FString> selectCliGhostResultsLines(const FCLIGhostState &State, const FGhostResults &Results) */
inline TArray<FString>
selectCliGhostResultsLines(const FCLIGhostState &State,
                           const FGhostResults &Results) {
  return func::concat_arrays<FString>({
      {Presentation::formatCliMessage(State.Results, Results.Passed,
                                      Results.TotalTests)},
      func::map_array<FGhostResultRecord, FString>(
          Results.Tests, [&State](const FGhostResultRecord &Result) {
            return Presentation::formatCliMessage(
                State.Result,
                Result.bTestPassed ? State.Passed : State.FailedTest,
                Result.TestName);
          }),
  });
}

/** User Story: As a QA operator, I need Ghost session progress selected from typed SDK state. @fn inline TArray<FString> selectCliGhostStatusLines(const FCLIGhostState &State, const FGhostStatus &Status) */
inline TArray<FString>
selectCliGhostStatusLines(const FCLIGhostState &State,
                          const FGhostStatus &Status) {
  return {
      Presentation::formatCliMessage(State.Session, Status.SessionId),
      Presentation::formatCliMessage(
          State.Status,
          Status.Status == State.CompletedStatus ? State.CompletedColor
                                                  : State.ActiveColor,
          Status.Status.ToUpper()),
      Presentation::formatCliMessage(State.Progress,
                                     FMath::RoundToInt(Status.Progress)),
      Presentation::formatCliMessage(State.Errors, Status.Errors),
      Presentation::formatCliMessage(State.Duration, Status.Duration),
  };
}

/** User Story: As a QA operator, I need recent Ghost sessions selected from typed history. @fn inline TArray<FString> selectCliGhostHistoryLines(const FCLIGhostState &State, const TArray<FGhostHistoryEntry> &Sessions) */
inline TArray<FString>
selectCliGhostHistoryLines(const FCLIGhostState &State,
                           const TArray<FGhostHistoryEntry> &Sessions) {
  return Sessions.Num() == State.EmptyCount
             ? TArray<FString>{State.HistoryEmpty}
             : func::map_array<FGhostHistoryEntry, FString>(
                   Sessions, [&State](const FGhostHistoryEntry &Session) {
                     return Presentation::formatCliMessage(
                         State.History, Session.SessionId,
                         Session.Status, Session.TestSuite, Session.PassRate);
                   });
}

} // namespace ForbocAI::CLI::Ghost
