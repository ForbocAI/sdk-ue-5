#pragma once

#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Quality/QualityAdapters.h"
#include "TestGame/Features/Systems/Quality/Text/TextAdapters.h"

namespace TestGame {
namespace QualityScoringAdaptersDetail {

/** User Story: As a quality report consumer, I need each metric represented by explicit pass evidence, score, and violations. @fn inline FQualityMetricEvaluation metricEvaluation(const TArray<FString> &Violations, double Score) */
inline FQualityMetricEvaluation
metricEvaluation(const TArray<FString> &Violations, double Score) {
  const TArray<FString> Unique = uniqueQualityValues(Violations);
  return {Unique.IsEmpty(), roundQualityNumber(Score), Unique};
}

/** User Story: As a pure evaluator, I need conditional violations expressed as immutable values. @fn inline TArray<FString> violationWhen(bool bViolated, const FString &Message) */
inline TArray<FString> violationWhen(bool bViolated,
                                     const FString &Message) {
  return bViolated ? TArray<FString>{Message} : TArray<FString>{};
}

/** User Story: As an instruction evaluator, I need authored token alternatives normalized before determining whether a requirement is met. @fn inline bool tokenGroupMissing(const TArray<FString> &Group, const FString &Response) */
inline bool tokenGroupMissing(const TArray<FString> &Group,
                              const FString &Response) {
  const FString NormalizedResponse = normalizeQualityText(Response);
  return !func::any_array<FString>(
      Group, [&NormalizedResponse](const FString &Token) {
        return NormalizedResponse.Contains(normalizeQualityText(Token));
      });
}

/** User Story: As an instruction evaluator, I need exact, semantic, token, and pattern requirements checked from one authored probe. @fn inline TArray<FString> expectationViolations(const FQualityProbe &Probe, const FString &Response, double ReferenceF1) */
inline TArray<FString> expectationViolations(const FQualityProbe &Probe,
                                             const FString &Response,
                                             double ReferenceF1) {
  const FQualityMessages &Messages = qualityData().Messages;
  const TArray<FString> MissingGroups = func::map_array<
      TArray<FString>, FString>(
      func::filter_array<TArray<FString>>(
          Probe.RequiredTokenGroups,
          [&Response](const TArray<FString> &Group) {
            return tokenGroupMissing(Group, Response);
          }),
      [&Messages](const TArray<FString> &) {
        return Messages.RequiredTokenGroupMissing;
      });
  const TArray<FString> MissingPatterns = func::map_array<FString, FString>(
      func::filter_array<FString>(
          Probe.RequiredPatterns,
          [&Response](const FString &Pattern) {
            return matchingQualityPatterns(Response, {Pattern}).IsEmpty();
          }),
      [&Messages](const FString &) { return Messages.RequiredPatternMissing; });
  const TArray<FString> ForbiddenPatterns = func::map_array<FString, FString>(
      func::filter_array<FString>(
          Probe.ForbiddenPatterns,
          [&Response](const FString &Pattern) {
            return !matchingQualityPatterns(Response, {Pattern}).IsEmpty();
          }),
      [&Messages](const FString &) {
        return Messages.ForbiddenPatternMatched;
      });
  return func::concat_arrays<FString>({
      violationWhen(!Probe.ExactResponse.IsEmpty() &&
                        normalizeQualityText(Response) !=
                            normalizeQualityText(Probe.ExactResponse),
                    Messages.ExactResponseMismatch),
      MissingGroups,
      MissingPatterns,
      ForbiddenPatterns,
      violationWhen(ReferenceF1 < Probe.MinimumReferenceF1,
                    Messages.ReferenceF1BelowMinimum),
  });
}

/**
 * User Story: As a quality evaluator, I need structural response failures derived independently of probe expectations.
 * @fn inline TArray<FString> structuralViolations( const FQualityProbe &Probe, const CommandRunner::FCommandOutput &Result, const FString &Response, int32 WordCount)
 */
inline TArray<FString> structuralViolations(
    const FQualityProbe &Probe, const CommandRunner::FCommandOutput &Result,
    const FString &Response, int32 WordCount) {
  const FQualityData &Data = qualityData();
  const int32 MaximumWords =
      FMath::Min(Probe.MaximumWords, Data.Numbers.MaximumResponseWords);
  return func::concat_arrays<FString>({
      violationWhen(Result.Status != Data.CommandStatuses.Ok,
                    Data.Messages.CommandFailed),
      violationWhen(Response.IsEmpty(), Data.Messages.DialogueMissing),
      violationWhen(WordCount < Data.Numbers.MinimumResponseWords,
                    Data.Messages.ResponseTooShort),
      violationWhen(WordCount > MaximumWords, Data.Messages.ResponseTooLong),
      violationWhen(!matchingQualityPatterns(Response,
                                             Data.Patterns.SystemLeaks)
                         .IsEmpty(),
                    Data.Messages.SystemPromptLeakage),
      violationWhen(!matchingQualityPatterns(Response,
                                             Data.Patterns.RoleEscapes)
                         .IsEmpty(),
                    Data.Messages.RoleEscape),
  });
}

/**
 * User Story: As a character evaluator, I need identity and unknown-boundary violations combined with structural evidence.
 * @fn inline TArray<FString> characterViolations( const FQualityProbe &Probe, const FString &Response, const TArray<FString> &Structural, const TArray<FString> &Expectations)
 */
inline TArray<FString> characterViolations(
    const FQualityProbe &Probe, const FString &Response,
    const TArray<FString> &Structural,
    const TArray<FString> &Expectations) {
  const FQualityData &Data = qualityData();
  return func::concat_arrays<FString>({
      Structural,
      violationWhen(!matchingQualityPatterns(
                          Response, Data.Patterns.SpecificSelfClaims)
                          .IsEmpty(),
                    Data.Messages.SpecificSelfClaim),
      violationWhen(!matchingQualityPatterns(
                          Response, Data.Patterns.IntrinsicIdentityClaims)
                          .IsEmpty(),
                    Data.Messages.IntrinsicIdentityClaim),
      violationWhen(Probe.bRequiresUnknownBoundary &&
                        matchingQualityPatterns(
                            Response, Data.Patterns.UnknownBoundary)
                            .IsEmpty(),
                    Data.Messages.UnknownBoundaryOmitted),
      Probe.Category == Data.Categories.CharacterConsistency
          ? Expectations
          : TArray<FString>{},
  });
}

} // namespace QualityScoringAdaptersDetail

/**
 * User Story: As a release evaluator, I need one pure scoring boundary for coherence, instruction following, character consistency, repetition, and latency.
 * @fn inline FQualitySample scoreQualitySample( const FQualityProbe &Probe, const CommandRunner::FCommandOutput &Result, const FQualityModelMetadata &Metadata, const FString &Host)
 */
inline FQualitySample scoreQualitySample(
    const FQualityProbe &Probe, const CommandRunner::FCommandOutput &Result,
    const FQualityModelMetadata &Metadata, const FString &Host) {
  using namespace QualityScoringAdaptersDetail;
  const FQualityData &Data = qualityData();
  const FString Response = extractQualityDialogue(Result.Output);
  const int32 WordCount = qualityWords(Response).Num();
  const double ReferenceF1 = referenceTokenF1(Probe.Reference, Response);
  const double RepetitionRatio = repeatedNgramRatio(Response);
  const TArray<FString> Expectations =
      expectationViolations(Probe, Response, ReferenceF1);
  const TArray<FString> Structural =
      structuralViolations(Probe, Result, Response, WordCount);
  const TArray<FString> Character =
      characterViolations(Probe, Response, Structural, Expectations);
  const TArray<FString> Repetition = func::concat_arrays<FString>({
      violationWhen(Result.Status != Data.CommandStatuses.Ok,
                    Data.Messages.CommandFailed),
      violationWhen(Response.IsEmpty(), Data.Messages.DialogueMissing),
      violationWhen(RepetitionRatio >
                        Data.Numbers.MaximumRepeatedNgramRatio,
                    Data.Messages.RepeatedNgramRatioExceeded),
  });
  const double LatencyBudgetMs =
      static_cast<double>(Metadata.InferenceLatencyBudgetMs) +
      Data.Hosts.FindChecked(Host).OverheadBudgetMs;
  const TArray<FString> Latency = func::concat_arrays<FString>({
      violationWhen(Result.DurationMs <= Data.Numbers.EmptyCount,
                    Data.Messages.LatencyMissing),
      violationWhen(Result.DurationMs > LatencyBudgetMs,
                    Data.Messages.LatencyBudgetExceeded),
  });
  TMap<FString, FQualityMetricEvaluation> Metrics;
  Metrics.Add(
      Data.Metrics.Coherence,
      metricEvaluation(
          func::concat_arrays<FString>({
              Structural,
              Probe.bCoherenceUsesExpectations ? Expectations
                                                : TArray<FString>{},
          }),
          ReferenceF1));
  Metrics.Add(Data.Metrics.InstructionFollowing,
              metricEvaluation(func::concat_arrays<FString>(
                                   {Structural, Expectations}),
                               ReferenceF1));
  Metrics.Add(Data.Metrics.CharacterConsistency,
              metricEvaluation(Character, ReferenceF1));
  Metrics.Add(Data.Metrics.Repetition,
              metricEvaluation(
                  Repetition,
                  Data.Numbers.SingularCount - RepetitionRatio));
  Metrics.Add(Data.Metrics.Latency,
              metricEvaluation(Latency, Result.DurationMs));
  return {Probe.Id,
          Probe.Category,
          Probe.PairKey,
          Probe.Command,
          Response,
          Probe.Reference,
          ReferenceF1,
          RepetitionRatio,
          Result.DurationMs,
          MoveTemp(Metrics)};
}

/** User Story: As a character consistency evaluator, I need repeated prompts compared without mistaking stable facts for undesirable phrase repetition. @fn inline FQualityPairComparison compareQualityPair(const TArray<FQualitySample> &Samples) */
inline FQualityPairComparison
compareQualityPair(const TArray<FQualitySample> &Samples) {
  const FQualityData &Data = qualityData();
  const bool bHasPair = Samples.Num() > Data.Numbers.SingularCount;
  const double Similarity = bHasPair
                                ? tokenJaccard(
                                      Samples[Data.Numbers.InitialIndex].Response,
                                      Samples[Data.Numbers.NextIndex].Response)
                                : Data.Numbers.MinimumReferenceF1;
  const bool bPassed =
      bHasPair && Similarity >= Data.Numbers.MinimumPairTokenJaccard;
  return {Samples.IsEmpty()
              ? Data.Output.Empty
              : Samples[Data.Numbers.InitialIndex].PairKey,
          func::map_array<FQualitySample, FString>(
              Samples, [](const FQualitySample &Sample) { return Sample.Id; }),
          Similarity,
          bPassed,
          bPassed ? TArray<FString>{}
                  : TArray<FString>{
                        Data.Messages.PairSimilarityBelowMinimum}};
}

} // namespace TestGame
