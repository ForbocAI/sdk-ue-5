#pragma once

#include "Core/fp.hpp"
#include "Components/CLI/CLITypes.h"
#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Components/Ghost/Results/ResultsTypes.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace ForbocAI::CLI::Ghost {

/** User Story: As a Ghost CLI consumer, I need feature state selected from the package CLI slice. @fn inline const FCLIGhostState &selectCliGhost(const FCLIState &State) */
inline const FCLIGhostState &selectCliGhost(const FCLIState &State) {
  return State.Ghost;
}

/**
 * User Story: As Ghost diagnostics, I need one consistent runtime attribution line across lifecycle projections.
 * @fn inline FString selectCliGhostRuntimeLine( const FCLIGhostState &State, const FGhostRuntimeIdentity &Identity)
 */
inline FString selectCliGhostRuntimeLine(
    const FCLIGhostState &State,
    const FGhostRuntimeIdentity &Identity) {
  return Presentation::formatCliMessage(
      State.Runtime, Identity.ApiVersion, Identity.SlmVersion,
      Identity.SlmStatus, Identity.SlotContractVersion);
}

/**
 * User Story: As Ghost diagnostics, I need API metrics rendered deterministically without deriving new conclusions.
 * @fn inline TArray<FString> selectCliGhostMetricLines( const FCLIGhostState &State, const TMap<FString, float> &Metrics)
 */
inline TArray<FString> selectCliGhostMetricLines(
    const FCLIGhostState &State,
    const TMap<FString, float> &Metrics) {
  TArray<FString> Names;
  Metrics.GenerateKeyArray(Names);
  Names.Sort();
  return func::map_array<FString, FString>(
      Names, [&State, &Metrics](const FString &Name) {
        return Presentation::formatCliMessage(
            State.Metric, Name, Metrics.FindRef(Name));
      });
}

/**
 * User Story: As a QA operator, I need API verdict, evidence, dimensions, checks, metrics, and summary rendered without SDK analysis.
 * @fn inline TArray<FString> selectCliGhostResultsLines( const FCLIGhostState &State, const FGhostResults &Results)
 */
inline TArray<FString> selectCliGhostResultsLines(
    const FCLIGhostState &State, const FGhostResults &Results) {
  return func::concat_arrays<FString>({
      {Presentation::formatCliMessage(State.Identity, Results.GhostName),
       selectCliGhostRuntimeLine(State, Results.RuntimeIdentity),
       Presentation::formatCliMessage(State.Results, Results.Passed,
                                      Results.TotalTests),
       Presentation::formatCliMessage(State.Verdict, Results.Verdict),
       Presentation::formatCliMessage(
           State.Coverage, FMath::RoundToInt(Results.Coverage)),
       Presentation::formatCliMessage(
           State.Evidence, Results.Evidence.CompletedProcessTurns,
           Results.Evidence.ValidCognitionOutcomes,
           Results.Evidence.InvalidCognitionOutcomes,
           Results.Evidence.ThoughtResults, Results.Evidence.ReasoningResults,
           Results.Evidence.ConsistentReasoningResults,
           Results.Evidence.DiagnosticResults,
           Results.Evidence.GroundedDiagnosticResults),
       Presentation::formatCliMessage(
           State.Integrity, Results.Evidence.SignedCognitionOutcomes,
           Results.Evidence.Contradictions,
           Results.Evidence.UnsupportedClaims, Results.Evidence.Failures,
           Results.Evidence.Timeouts),
       Presentation::formatCliMessage(
           State.Exploration, Results.Evidence.UniqueObservations,
           Results.Evidence.ChoicePoints),
       Presentation::formatCliMessage(
           State.EvidenceDimensions,
           FString::Join(Results.EvidenceDimensions,
                         *State.DimensionSeparator)),
       Presentation::formatCliMessage(
           State.EvaluationDimensions,
           FString::Join(Results.EvaluationDimensions,
                         *State.DimensionSeparator))},
      func::map_array<FGhostResultRecord, FString>(
          Results.Tests, [&State](const FGhostResultRecord &Result) {
            return Presentation::formatCliMessage(
                State.Result,
                Result.bTestPassed ? State.Passed : State.FailedTest,
                Result.TestName);
          }),
      selectCliGhostMetricLines(State, Results.Metrics),
      {Presentation::formatCliMessage(State.Summary, Results.Summary)},
  });
}

/**
 * User Story: As a QA operator, I need Ghost status include API identity, suite, progress, errors, and duration.
 * @fn inline TArray<FString> selectCliGhostStatusLines( const FCLIGhostState &State, const FGhostStatus &Status)
 */
inline TArray<FString> selectCliGhostStatusLines(
    const FCLIGhostState &State, const FGhostStatus &Status) {
  return {
      Presentation::formatCliMessage(State.Identity, Status.GhostName),
      selectCliGhostRuntimeLine(State, Status.RuntimeIdentity),
      Presentation::formatCliMessage(State.Suite, Status.TestSuite),
      Presentation::formatCliMessage(State.Session, Status.SessionId),
      Presentation::formatCliMessage(
          State.Status,
          Status.Status == State.CompletedStatus ? State.CompletedColor
                                                  : State.ActiveColor,
          Status.Status.ToUpper()),
      Presentation::formatCliMessage(State.Progress, Status.Progress),
      Presentation::formatCliMessage(State.Errors, Status.Errors),
      Presentation::formatCliMessage(State.Duration, Status.Duration),
  };
}

/**
 * User Story: As a QA operator, I need one history entry preserve API identity and runtime attribution.
 * @fn inline TArray<FString> selectCliGhostHistoryEntryLines( const FCLIGhostState &State, const FGhostHistoryEntry &Session)
 */
inline TArray<FString> selectCliGhostHistoryEntryLines(
    const FCLIGhostState &State, const FGhostHistoryEntry &Session) {
  return {
      Presentation::formatCliMessage(
          State.History, Session.SessionId, Session.Status,
          Session.GhostName, Session.TestSuite, Session.PassRate),
      Presentation::formatCliMessage(
          State.HistoryRuntime, Session.RuntimeIdentity.ApiVersion,
          Session.RuntimeIdentity.SlmVersion,
          Session.RuntimeIdentity.SlmStatus,
          Session.RuntimeIdentity.SlotContractVersion),
  };
}

/**
 * User Story: As a QA operator, I need recent Ghost sessions selected from typed API history.
 * @fn inline TArray<FString> selectCliGhostHistoryLines( const FCLIGhostState &State, const TArray<FGhostHistoryEntry> &Sessions)
 */
inline TArray<FString> selectCliGhostHistoryLines(
    const FCLIGhostState &State,
    const TArray<FGhostHistoryEntry> &Sessions) {
  return Sessions.Num() == State.EmptyCount
             ? TArray<FString>{State.HistoryEmpty}
             : func::fold_array<FGhostHistoryEntry, TArray<FString>>(
                   Sessions, TArray<FString>(),
                   [&State](const TArray<FString> &Lines,
                            const FGhostHistoryEntry &Session) {
                     return func::concat_arrays<FString>(
                         {Lines,
                          selectCliGhostHistoryEntryLines(State, Session)});
                   });
}

} // namespace ForbocAI::CLI::Ghost
