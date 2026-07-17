#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptSelectors.h"

namespace TestGame::GameSelectors {

/** User Story: As a harness maintainer, I need run completeness derived from root-store coverage, transcript, and active API contract state so no duplicated completion flag can drift. @fn inline FGameRunResult SelectGameRunResult(const FTestGameState &State) */
inline FGameRunResult SelectGameRunResult(const FTestGameState &State) {
  FGameRunResult Result;
  Result.MissingGroups = CoverageSelectors::SelectHarnessMissingGroups(
      State.Harness, ScenarioSelectors::SelectRequiredCommandGroups(
                         State.Scenario));
  Result.Transcript =
      TranscriptSelectors::SelectTranscriptEntries(State.Transcript);
  Result.TranscriptErrorCount =
      TranscriptSelectors::SelectTranscriptErrorCount(State.Transcript);
  Result.bComplete =
      Result.MissingGroups.Num() ==
          GameAdapters::GameRuntimeData().numbers.emptyCount &&
      Result.TranscriptErrorCount ==
          GameAdapters::GameRuntimeData().numbers.emptyCount;
  return Result;
}

/** User Story: As a harness presenter, I need summary text projected from derived run state and authored templates so no presentation strings live in behavior code. @fn inline FString SelectGameSummaryText(const FGameRunResult &Result) */
inline FString SelectGameSummaryText(const FGameRunResult &Result) {
  const FGameRuntimeData &Data = GameAdapters::GameRuntimeData();
  return Result.bComplete
             ? Data.messages.coverageComplete
             : [&]() {
                 TMap<FString, FString> ErrorValues;
                 ErrorValues.Add(
                     Data.tokens.count,
                     FString::FromInt(Result.TranscriptErrorCount));
                 ErrorValues.Add(
                     Data.tokens.suffix,
                     Result.TranscriptErrorCount == Data.numbers.singularCount
                         ? Data.messages.singularSuffix
                         : Data.messages.pluralSuffix);
                 const FString Errors = GameAdapters::FormatGameTemplate(
                     Data.messages.coverageErrors, ErrorValues);

                 TMap<FString, FString> MissingValues;
                 MissingValues.Add(
                     Data.tokens.groups,
                     FString::Join(Result.MissingGroups,
                                   *Data.separators.list));
                 const FString Missing =
                     Result.MissingGroups.Num() > Data.numbers.emptyCount
                         ? GameAdapters::FormatGameTemplate(
                               Data.messages.missingGroups, MissingValues)
                         : Data.messages.noMissingGroups;

                 TMap<FString, FString> SummaryValues;
                 SummaryValues.Add(Data.tokens.errors, Errors);
                 SummaryValues.Add(Data.tokens.missing, Missing);
                 return GameAdapters::FormatGameTemplate(
                     Data.messages.coverageIncomplete, SummaryValues);
               }();
}

} // namespace TestGame::GameSelectors
