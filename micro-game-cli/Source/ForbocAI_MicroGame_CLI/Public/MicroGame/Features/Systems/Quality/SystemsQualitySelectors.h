#pragma once

#include "MicroGame/Features/Systems/Quality/Baseline/BaselineAdapters.h"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"
#include "MicroGame/Features/Systems/Quality/Scoring/ScoringAdapters.h"

namespace MicroGame {
namespace QualitySelectorsDetail {

/** User Story: As a quality selector, I need normalized samples ordered by stable probe identity. @fn inline TArray<FQualitySample> samples(const FQualityState &State) */
inline TArray<FQualitySample> samples(const FQualityState &State) {
  TArray<FString> Keys;
  State.Samples.GetKeys(Keys);
  Keys.Sort();
  return func::map_array<FString, FQualitySample>(
      Keys, [&State](const FString &Key) {
        return State.Samples.FindChecked(Key);
      });
}

/** User Story: As a consistency evaluator, I need repeated-prompt samples grouped by authored pair identity. @fn inline TArray<TArray<FQualitySample>> pairGroups(const TArray<FQualitySample> &Samples) */
inline TArray<TArray<FQualitySample>>
pairGroups(const TArray<FQualitySample> &Samples) {
  const TMap<FString, TArray<FQualitySample>> Groups =
      func::fold_array<FQualitySample,
                       TMap<FString, TArray<FQualitySample>>>(
          Samples, {}, [](const TMap<FString, TArray<FQualitySample>> &Acc,
                          const FQualitySample &Sample) {
            TMap<FString, TArray<FQualitySample>> Next = Acc;
            return Sample.PairKey.IsEmpty()
                       ? Next
                       : (Next.FindOrAdd(Sample.PairKey).Add(Sample), Next);
          });
  TArray<FString> Keys;
  Groups.GetKeys(Keys);
  Keys.Sort();
  return func::map_array<FString, TArray<FQualitySample>>(
      Keys, [&Groups](const FString &Key) {
        return Groups.FindChecked(Key);
      });
}

/** User Story: As a character consistency reviewer, I need repeated-prompt agreement derived from normalized samples. @fn inline TArray<FQualityPairComparison> pairComparisons(const TArray<FQualitySample> &Samples) */
inline TArray<FQualityPairComparison>
pairComparisons(const TArray<FQualitySample> &Samples) {
  return func::map_array<TArray<FQualitySample>, FQualityPairComparison>(
      pairGroups(Samples), compareQualityPair);
}

/**
 * User Story: As a quality reviewer, I need sample and pair evidence combined only for the metric that owns it.
 * @fn inline TArray<FQualityMetricEvaluation> metricEvidence( const FString &Metric, const TArray<FQualitySample> &Samples, const TArray<FQualityPairComparison> &Pairs)
 */
inline TArray<FQualityMetricEvaluation> metricEvidence(
    const FString &Metric, const TArray<FQualitySample> &Samples,
    const TArray<FQualityPairComparison> &Pairs) {
  return func::concat_arrays<FQualityMetricEvaluation>({
      func::map_array<FQualitySample, FQualityMetricEvaluation>(
          Samples, [&Metric](const FQualitySample &Sample) {
            return Sample.Metrics.FindChecked(Metric);
          }),
      Metric == qualityData().Metrics.CharacterConsistency
          ? func::map_array<FQualityPairComparison,
                            FQualityMetricEvaluation>(
                Pairs, [](const FQualityPairComparison &Pair) {
                  return FQualityMetricEvaluation{
                      Pair.bPassed, Pair.TokenJaccard, Pair.Violations};
                })
          : TArray<FQualityMetricEvaluation>{},
  });
}

/**
 * User Story: As a release gate, I need every quality metric summarized with an explicit nonempty denominator.
 * @fn inline FQualityMetricSummary summarizeMetric( const FString &Metric, const TArray<FQualitySample> &Samples, const TArray<FQualityPairComparison> &Pairs)
 */
inline FQualityMetricSummary summarizeMetric(
    const FString &Metric, const TArray<FQualitySample> &Samples,
    const TArray<FQualityPairComparison> &Pairs) {
  const TArray<FQualityMetricEvaluation> Evidence =
      metricEvidence(Metric, Samples, Pairs);
  const int32 Passed =
      func::filter_array<FQualityMetricEvaluation>(
          Evidence, [](const FQualityMetricEvaluation &Entry) {
            return Entry.bPassed;
          })
          .Num();
  const int32 Total = Evidence.Num();
  return {Passed,
          Total,
          Total == qualityData().Numbers.EmptyCount
              ? qualityData().Numbers.MinimumReferenceF1
              : roundQualityNumber(static_cast<double>(Passed) /
                                   static_cast<double>(Total)),
          Total > qualityData().Numbers.EmptyCount && Passed == Total,
          func::concat_arrays<FString>(
              func::map_array<FQualityMetricEvaluation, TArray<FString>>(
                  Evidence,
                  [](const FQualityMetricEvaluation &Entry) {
                    return Entry.Violations;
                  }))};
}

/** User Story: As a latency evaluator, I need missing timings excluded from aggregates while their sample metric remains failed. @fn inline TArray<double> latencyValues(const TArray<FQualitySample> &Samples) */
inline TArray<double> latencyValues(const TArray<FQualitySample> &Samples) {
  TArray<double> Values = func::filter_array<double>(
      func::map_array<FQualitySample, double>(
          Samples,
          [](const FQualitySample &Sample) { return Sample.DurationMs; }),
      [](double Value) { return Value > qualityData().Numbers.EmptyCount; });
  Values.Sort();
  return Values;
}

/** User Story: As a release reviewer, I need mean command latency derived from every timed model probe. @fn inline double meanLatency(const TArray<double> &Values) */
inline double meanLatency(const TArray<double> &Values) {
  return Values.IsEmpty()
             ? qualityData().Numbers.MinimumReferenceF1
             : roundQualityNumber(
                   func::fold_array<double, double>(
                       Values, qualityData().Numbers.MinimumReferenceF1,
                       [](double Total, double Value) {
                         return Total + Value;
                       }) /
                   static_cast<double>(Values.Num()));
}

/** User Story: As a release reviewer, I need tail latency represented without allowing one fast response to conceal slow interactions. @fn inline double percentileLatency(const TArray<double> &Values) */
inline double percentileLatency(const TArray<double> &Values) {
  const int32 Index = FMath::Min(
      Values.Num() - qualityData().Numbers.NextIndex,
      FMath::CeilToInt(Values.Num() * qualityData().Numbers.LatencyPercentile) -
          qualityData().Numbers.NextIndex);
  return Values.IsEmpty()
             ? qualityData().Numbers.MinimumReferenceF1
             : roundQualityNumber(Values[Index]);
}

/** User Story: As a before/after evaluator, I need incompatible reports and changed workloads rejected rather than silently compared. @fn inline FString baselineStatus(const FQualityState &State, const TArray<FQualitySample> &Samples) */
inline FString baselineStatus(const FQualityState &State,
                              const TArray<FQualitySample> &Samples) {
  return !State.Baseline.hasValue
             ? qualityData().BaselineStatuses.Missing
         : !State.Host.IsEmpty() &&
                   State.Baseline.value.SchemaVersion ==
                       qualityData().SchemaVersion &&
                   State.Baseline.value.ContractVersion ==
                       qualityData().ContractVersion &&
                   State.Baseline.value.EvaluationScope ==
                       qualityData().EvaluationScope &&
                   State.Baseline.value.Host == State.Host &&
                   qualityWorkloadMatches(Samples, State.Baseline.value)
             ? qualityData().BaselineStatuses.Compatible
             : qualityData().BaselineStatuses.Incompatible;
}

/**
 * User Story: As a release reviewer, I need each current pass rate compared against the exact compatible parent contract.
 * @fn inline TArray<FQualityRegression> metricRegressions( const TMap<FString, FQualityMetricSummary> &Metrics, const FQualityState &State, const FString &Status)
 */
inline TArray<FQualityRegression> metricRegressions(
    const TMap<FString, FQualityMetricSummary> &Metrics,
    const FQualityState &State, const FString &Status) {
  return Status != qualityData().BaselineStatuses.Compatible ||
                 !State.Baseline.hasValue
             ? TArray<FQualityRegression>{}
             : func::map_array<FString, FQualityRegression>(
                   qualityData().MetricNames,
                   [&Metrics, &State](const FString &Metric) {
                     const double Current =
                         Metrics.FindChecked(Metric).PassRate;
                     const double Baseline = State.Baseline.value.Summary.Metrics
                                                 .FindChecked(Metric)
                                                 .PassRate;
                     return FQualityRegression{
                         Metric,
                         qualityData().RegressionMeasures.PassRate,
                         Baseline,
                         Current,
                         Current + qualityData()
                                       .Numbers.RateRegressionTolerance <
                             Baseline};
                   });
}

/**
 * User Story: As a player-experience reviewer, I need latency regression require both a relative slowdown and host-sized absolute evidence so transport jitter cannot create a false failure.
 * @fn inline TArray<FQualityRegression> latencyRegression( double MeanLatencyMs, const FQualityState &State, const FString &Status)
 */
inline TArray<FQualityRegression> latencyRegression(
    double MeanLatencyMs, const FQualityState &State,
    const FString &Status) {
  return Status != qualityData().BaselineStatuses.Compatible ||
                 !State.Baseline.hasValue
             ? TArray<FQualityRegression>{}
             : TArray<FQualityRegression>{FQualityRegression{
                   qualityData().Metrics.Latency,
                   qualityData().RegressionMeasures.MeanLatencyMs,
                   State.Baseline.value.Summary.MeanLatencyMs,
                   MeanLatencyMs,
                   MeanLatencyMs >
                           State.Baseline.value.Summary.MeanLatencyMs *
                               (qualityData().Numbers.SingularCount +
                                qualityData()
                                    .Numbers.LatencyRegressionToleranceRatio) &&
                       MeanLatencyMs -
                               State.Baseline.value.Summary.MeanLatencyMs >
                           qualityData()
                               .Hosts.FindChecked(State.Host)
                               .OverheadBudgetMs}};
}

} // namespace QualitySelectorsDetail

/** User Story: As a release gate, I need absolute quality and before/after regression decisions derived from normalized state. @fn inline FQualityReportSummary selectQualityReportSummary(const FQualityState &State) */
inline FQualityReportSummary
selectQualityReportSummary(const FQualityState &State) {
  using namespace QualitySelectorsDetail;
  const TArray<FQualitySample> Samples = samples(State);
  const TArray<FQualityPairComparison> Pairs = pairComparisons(Samples);
  const TMap<FString, FQualityMetricSummary> Metrics =
      func::fold_array<FString, TMap<FString, FQualityMetricSummary>>(
          qualityData().MetricNames, {},
          [&Samples, &Pairs](const TMap<FString, FQualityMetricSummary> &Acc,
                             const FString &Metric) {
            TMap<FString, FQualityMetricSummary> Next = Acc;
            Next.Add(Metric, summarizeMetric(Metric, Samples, Pairs));
            return Next;
          });
  const TArray<double> Latencies = latencyValues(Samples);
  const double MeanLatencyMs = meanLatency(Latencies);
  const FString BaselineStatus = baselineStatus(State, Samples);
  const TArray<FQualityRegression> Regressions =
      func::concat_arrays<FQualityRegression>({
          metricRegressions(Metrics, State, BaselineStatus),
          latencyRegression(MeanLatencyMs, State, BaselineStatus),
      });
  const bool bAbsoluteGatePassed =
      State.Status == qualityData().RunStatuses.Completed &&
      func::all_array<FString>(
          qualityData().MetricNames, [&Metrics](const FString &Metric) {
            return Metrics.FindChecked(Metric).bGatePassed;
          });
  const bool bRegressionGatePassed =
      BaselineStatus == qualityData().BaselineStatuses.Compatible &&
      func::all_array<FQualityRegression>(
          Regressions, [](const FQualityRegression &Regression) {
            return !Regression.bRegressed;
          });
  return {Metrics,
          Pairs,
          MeanLatencyMs,
          percentileLatency(Latencies),
          bAbsoluteGatePassed,
          BaselineStatus,
          Regressions,
          bRegressionGatePassed,
          bAbsoluteGatePassed && bRegressionGatePassed};
}

/** User Story: As a release operator, I need a versioned persisted report bound to one host, model artifact, and quality contract. @fn inline func::Maybe<FQualityReport> selectQualityReport(const FQualityState &State) */
inline func::Maybe<FQualityReport>
selectQualityReport(const FQualityState &State) {
  return State.bRequired && !State.Host.IsEmpty() &&
                 !State.CompletedAt.IsEmpty() && State.Metadata.hasValue
             ? func::just(FQualityReport{
                   qualityData().SchemaVersion,
                   qualityData().ContractVersion,
                   qualityData().EvaluationScope,
                   State.Host,
                   State.CompletedAt,
                   State.Metadata.value,
                   State.Error,
                   selectQualityReportSummary(State),
                   QualitySelectorsDetail::samples(State)})
             : func::nothing<FQualityReport>();
}

/** User Story: As a game completion selector, I need custom contracts unaffected while canonical live runs require the quality gate. @fn inline bool selectQualityGatePassed(const FQualityState &State) */
inline bool selectQualityGatePassed(const FQualityState &State) {
  return !State.bRequired ||
         selectQualityReportSummary(State).bQualityGatePassed;
}

} // namespace MicroGame
