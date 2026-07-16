#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptSelectors.h"

namespace TestGame {
namespace GameSelectors {

/** User Story: As a systems harness game consumer, I need to invoke select game run result through a stable signature so the systems harness game workflow remains explicit and composable. @fn inline FGameRunResult SelectGameRunResult( const FTestGameState &State, const TArray<ECommandGroup> &RequiredGroups) */
inline FGameRunResult SelectGameRunResult(
    const FTestGameState &State,
    const TArray<ECommandGroup> &RequiredGroups) {
  FGameRunResult Result;
  Result.MissingGroups = HarnessSelectors::SelectHarnessMissingGroups(
      State.Harness, RequiredGroups);
  Result.Transcript =
      TranscriptSelectors::SelectTranscriptEntries(State.Transcript);
  const int32 ErrorCount =
      TranscriptSelectors::SelectTranscriptErrorCount(State.Transcript);
  Result.bComplete = Result.MissingGroups.Num() == 0 && ErrorCount == 0;
  Result.Summary = Result.bComplete
                       ? FString(TEXT("CLI coverage complete."))
                       : FString::Printf(
                             TEXT("CLI coverage incomplete: %d transcript "
                                  "error%s; %d missing command group%s."),
                             ErrorCount, ErrorCount == 1 ? TEXT("") : TEXT("s"),
                             Result.MissingGroups.Num(),
                             Result.MissingGroups.Num() == 1 ? TEXT("")
                                                             : TEXT("s"));
  return Result;
}

} // namespace GameSelectors
} // namespace TestGame
