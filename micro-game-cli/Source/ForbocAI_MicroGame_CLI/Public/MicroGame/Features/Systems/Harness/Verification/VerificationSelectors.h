#pragma once

#include "MicroGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "MicroGame/Features/Systems/Quality/SystemsQualitySelectors.h"
#include "MicroGame/Features/Systems/Terminal/Transcript/TranscriptSelectors.h"
#include "MicroGame/Features/Systems/Terminal/UI/UISelectors.h"

namespace MicroGame::VerificationSelectors {

/** User Story: As a harness maintainer, I need run completeness derived from root-store coverage, transcript, and active API contract state so no duplicated completion flag can drift. @fn inline FGameRunResult SelectGameRunResult(const FMicroGameState &State) */
inline FGameRunResult SelectGameRunResult(const FMicroGameState &State) {
  FGameRunResult Result;
  const bool bChatOnly =
      UISelectors::SelectUiMode(State.UI) ==
      VerificationVocabularyAdapters::GameRuntimeData().modes.chat;
  const TArray<FString> RequiredGroups =
      bChatOnly
          ? TArray<FString>{VerificationVocabularyAdapters::GameRuntimeData()
                                .commandGroups.npc_conversation}
          : ScenarioSelectors::SelectRequiredCommandGroups(State.Scenario);
  Result.MissingGroups = CoverageSelectors::SelectHarnessMissingGroups(
      State.Harness, RequiredGroups);
  Result.Transcript =
      TranscriptSelectors::SelectTranscriptEntries(State.Transcript);
  Result.TranscriptErrorCount =
      TranscriptSelectors::SelectTranscriptErrorCount(State.Transcript);
  Result.bQualityRequired = State.Quality.bRequired;
  Result.bQualityGatePassed = selectQualityGatePassed(State.Quality);
  Result.QualityReport = selectQualityReport(State.Quality);
  Result.bComplete =
      Result.MissingGroups.Num() ==
          VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount &&
      Result.TranscriptErrorCount ==
          VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount &&
      Result.bQualityGatePassed;
  return Result;
}

/** User Story: As a harness presenter, I need summary text projected from derived run state and authored templates so no presentation strings live in behavior code. @fn inline FString SelectGameSummaryText(const FGameRunResult &Result) */
inline FString SelectGameSummaryText(const FGameRunResult &Result) {
  const FGameRuntimeData &Data = VerificationVocabularyAdapters::GameRuntimeData();
  return Result.bComplete
             ? (Result.bQualityRequired
                    ? Data.messages.coverageComplete +
                          qualityData().Output.Space +
                          qualityData().Messages.QualityPassed
                    : Data.messages.coverageComplete)
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
                 const FString Errors = VerificationAdapters::FormatGameTemplate(
                     Data.messages.coverageErrors, ErrorValues);

                 const FString QualityError =
                     !Result.bQualityRequired || Result.bQualityGatePassed
                         ? qualityData().Output.Empty
                     : Result.QualityReport.hasValue &&
                               !Result.QualityReport.value.Summary
                                    .bAbsoluteGatePassed
                         ? qualityData().Messages.AbsoluteGateFailed
                     : Result.QualityReport.hasValue &&
                               Result.QualityReport.value.Summary
                                       .BaselineStatus ==
                                   qualityData().BaselineStatuses.Missing
                         ? qualityData().Messages.BaselineMissing
                     : Result.QualityReport.hasValue &&
                               Result.QualityReport.value.Summary
                                       .BaselineStatus ==
                                   qualityData().BaselineStatuses.Incompatible
                         ? qualityData().Messages.BaselineIncompatible
                         : qualityData().Messages.RegressionGateFailed;
                 const FString CombinedErrors =
                     QualityError.IsEmpty()
                         ? Errors
                         : Errors + Data.separators.list + QualityError;

                 TMap<FString, FString> MissingValues;
                 MissingValues.Add(
                     Data.tokens.groups,
                     FString::Join(Result.MissingGroups,
                                   *Data.separators.list));
                 const FString Missing =
                     Result.MissingGroups.Num() > Data.numbers.emptyCount
                         ? VerificationAdapters::FormatGameTemplate(
                               Data.messages.missingGroups, MissingValues)
                         : Data.messages.noMissingGroups;

                 TMap<FString, FString> SummaryValues;
                 SummaryValues.Add(Data.tokens.errors, CombinedErrors);
                 SummaryValues.Add(Data.tokens.missing, Missing);
                 return VerificationAdapters::FormatGameTemplate(
                     Data.messages.coverageIncomplete, SummaryValues);
               }();
}

} // namespace MicroGame::VerificationSelectors
