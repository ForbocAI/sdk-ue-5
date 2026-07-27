#pragma once

#include "Serialization/JsonSerializer.h"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"

namespace MicroGame {
namespace QualityReportSerializationAdaptersDetail {

/** User Story: As a report serializer, I need strings converted to JSON values through one reusable collection adapter. @fn inline TArray<TSharedPtr<FJsonValue>> stringValues(const TArray<FString> &Values) */
inline TArray<TSharedPtr<FJsonValue>>
stringValues(const TArray<FString> &Values) {
  return func::map_array<FString, TSharedPtr<FJsonValue>>(
      Values, [](const FString &Value) {
        return MakeShared<FJsonValueString>(Value);
      });
}

/** User Story: As a report serializer, I need object arrays encoded without feature-local traversal. @fn inline TArray<TSharedPtr<FJsonValue>> objectValues(const TArray<TSharedRef<FJsonObject>> &Values) */
inline TArray<TSharedPtr<FJsonValue>>
objectValues(const TArray<TSharedRef<FJsonObject>> &Values) {
  return func::map_array<TSharedRef<FJsonObject>, TSharedPtr<FJsonValue>>(
      Values, [](const TSharedRef<FJsonObject> &Value) {
        return MakeShared<FJsonValueObject>(Value);
      });
}

/** User Story: As a report consumer, I need every metric observation serialized with score and violation evidence. @fn inline TSharedRef<FJsonObject> metricEvaluationObject(const FQualityMetricEvaluation &Evaluation) */
inline TSharedRef<FJsonObject>
metricEvaluationObject(const FQualityMetricEvaluation &Evaluation) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteBooleanField(Object, TEXT("passed"),
                                  Evaluation.bPassed);
  DataAdapters::WriteNumberField(Object, TEXT("score"), Evaluation.Score);
  DataAdapters::WriteArrayField(Object, TEXT("violations"),
                                stringValues(Evaluation.Violations));
  return Object;
}

/** User Story: As a report consumer, I need aggregate metric gates persisted with their exact denominator and evidence. @fn inline TSharedRef<FJsonObject> metricSummaryObject(const FQualityMetricSummary &Summary) */
inline TSharedRef<FJsonObject>
metricSummaryObject(const FQualityMetricSummary &Summary) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteNumberField(Object, TEXT("passed"), Summary.Passed);
  DataAdapters::WriteNumberField(Object, TEXT("total"), Summary.Total);
  DataAdapters::WriteNumberField(Object, TEXT("passRate"), Summary.PassRate);
  DataAdapters::WriteBooleanField(Object, TEXT("gatePassed"),
                                  Summary.bGatePassed);
  DataAdapters::WriteArrayField(Object, TEXT("violations"),
                                stringValues(Summary.Violations));
  return Object;
}

/** User Story: As a release reviewer, I need each report bound to the exact API and SLM artifact under test. @fn inline TSharedRef<FJsonObject> metadataObject(const FQualityModelMetadata &Metadata) */
inline TSharedRef<FJsonObject>
metadataObject(const FQualityModelMetadata &Metadata) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteStringField(Object, TEXT("apiStatus"),
                                 Metadata.ApiStatus);
  DataAdapters::WriteStringField(Object, TEXT("apiVersion"),
                                 Metadata.ApiVersion);
  DataAdapters::WriteNumberField(Object, TEXT("inferenceLatencyBudgetMs"),
                                 Metadata.InferenceLatencyBudgetMs);
  DataAdapters::WriteStringField(Object, TEXT("slmStatus"),
                                 Metadata.SlmStatus);
  DataAdapters::WriteStringField(Object, TEXT("slmVersion"),
                                 Metadata.SlmVersion);
  DataAdapters::WriteStringField(Object, TEXT("slotContractVersion"),
                                 Metadata.SlotContractVersion);
  DataAdapters::WriteStringField(Object, TEXT("slmArtifactSha256"),
                                 Metadata.SlmArtifactSha256);
  return Object;
}

/** User Story: As a consistency reviewer, I need pairwise evidence persisted beside per-sample metrics. @fn inline TSharedRef<FJsonObject> pairObject(const FQualityPairComparison &Pair) */
inline TSharedRef<FJsonObject>
pairObject(const FQualityPairComparison &Pair) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteStringField(Object, TEXT("pairKey"), Pair.PairKey);
  DataAdapters::WriteArrayField(Object, TEXT("sampleIds"),
                                stringValues(Pair.SampleIds));
  DataAdapters::WriteNumberField(Object, TEXT("tokenJaccard"),
                                 Pair.TokenJaccard);
  DataAdapters::WriteBooleanField(Object, TEXT("passed"), Pair.bPassed);
  DataAdapters::WriteArrayField(Object, TEXT("violations"),
                                stringValues(Pair.Violations));
  return Object;
}

/** User Story: As a before-and-after reviewer, I need every compared measure persisted with current and accepted values. @fn inline TSharedRef<FJsonObject> regressionObject(const FQualityRegression &Regression) */
inline TSharedRef<FJsonObject>
regressionObject(const FQualityRegression &Regression) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteStringField(Object, TEXT("metric"), Regression.Metric);
  DataAdapters::WriteStringField(Object, TEXT("measure"), Regression.Measure);
  DataAdapters::WriteNumberField(Object, TEXT("baselineValue"),
                                 Regression.BaselineValue);
  DataAdapters::WriteNumberField(Object, TEXT("currentValue"),
                                 Regression.CurrentValue);
  DataAdapters::WriteBooleanField(Object, TEXT("regressed"),
                                  Regression.bRegressed);
  return Object;
}

/** User Story: As a quality reviewer, I need every live probe serialized with raw dialogue, timing, and metric evidence. @fn inline TSharedRef<FJsonObject> sampleObject(const FQualitySample &Sample) */
inline TSharedRef<FJsonObject> sampleObject(const FQualitySample &Sample) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteStringField(Object, TEXT("id"), Sample.Id);
  DataAdapters::WriteStringField(Object, TEXT("category"), Sample.Category);
  DataAdapters::WriteStringField(Object, TEXT("pairKey"), Sample.PairKey);
  DataAdapters::WriteStringField(Object, TEXT("command"), Sample.Command);
  DataAdapters::WriteStringField(Object, TEXT("response"), Sample.Response);
  DataAdapters::WriteStringField(Object, TEXT("reference"), Sample.Reference);
  DataAdapters::WriteNumberField(Object, TEXT("referenceTokenF1"),
                                 Sample.ReferenceTokenF1);
  DataAdapters::WriteNumberField(Object, TEXT("repeatedNgramRatio"),
                                 Sample.RepeatedNgramRatio);
  DataAdapters::WriteNumberField(Object, TEXT("durationMs"),
                                 Sample.DurationMs);
  const TSharedRef<FJsonObject> Metrics = MakeShared<FJsonObject>();
  TArray<FString> MetricNames;
  Sample.Metrics.GetKeys(MetricNames);
  MetricNames.Sort();
  func::for_each_array<FString>(MetricNames, [&Sample, &Metrics](
                                                 const FString &Metric) {
    DataAdapters::WriteObjectField(
        Metrics, Metric,
        metricEvaluationObject(Sample.Metrics.FindChecked(Metric)));
  });
  DataAdapters::WriteObjectField(Object, TEXT("metrics"), Metrics);
  return Object;
}

/** User Story: As a before-and-after evaluator, I need comparison-authoritative sample fields restored from the committed baseline. @fn inline FQualitySample baselineSample(const TSharedPtr<FJsonValue> &Value) */
inline FQualitySample baselineSample(const TSharedPtr<FJsonValue> &Value) {
  const TSharedRef<FJsonObject> Object = Value->AsObject().ToSharedRef();
  return {DataAdapters::ReadStringField(Object, TEXT("id")),
          DataAdapters::ReadStringField(Object, TEXT("category")),
          DataAdapters::ReadStringField(Object, TEXT("pairKey")),
          DataAdapters::ReadStringField(Object, TEXT("command")),
          DataAdapters::ReadStringField(Object, TEXT("response")),
          DataAdapters::ReadStringField(Object, TEXT("reference")),
          DataAdapters::ReadDoubleField(Object, TEXT("referenceTokenF1")),
          DataAdapters::ReadDoubleField(Object, TEXT("repeatedNgramRatio")),
          DataAdapters::ReadDoubleField(Object, TEXT("durationMs")),
          {}};
}

/** User Story: As a release gate consumer, I need aggregate decisions serialized independently from rendering. @fn inline TSharedRef<FJsonObject> summaryObject(const FQualityReportSummary &Summary) */
inline TSharedRef<FJsonObject>
summaryObject(const FQualityReportSummary &Summary) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  const TSharedRef<FJsonObject> Metrics = MakeShared<FJsonObject>();
  TArray<FString> MetricNames;
  Summary.Metrics.GetKeys(MetricNames);
  MetricNames.Sort();
  func::for_each_array<FString>(MetricNames, [&Summary, &Metrics](
                                                 const FString &Metric) {
    DataAdapters::WriteObjectField(
        Metrics, Metric,
        metricSummaryObject(Summary.Metrics.FindChecked(Metric)));
  });
  DataAdapters::WriteObjectField(Object, TEXT("metrics"), Metrics);
  DataAdapters::WriteArrayField(
      Object, TEXT("pairComparisons"),
      objectValues(func::map_array<FQualityPairComparison,
                                   TSharedRef<FJsonObject>>(
          Summary.PairComparisons, pairObject)));
  DataAdapters::WriteNumberField(Object, TEXT("meanLatencyMs"),
                                 Summary.MeanLatencyMs);
  DataAdapters::WriteNumberField(Object, TEXT("percentileLatencyMs"),
                                 Summary.PercentileLatencyMs);
  DataAdapters::WriteBooleanField(Object, TEXT("absoluteGatePassed"),
                                  Summary.bAbsoluteGatePassed);
  DataAdapters::WriteStringField(Object, TEXT("baselineStatus"),
                                 Summary.BaselineStatus);
  DataAdapters::WriteArrayField(
      Object, TEXT("regressions"),
      objectValues(func::map_array<FQualityRegression,
                                   TSharedRef<FJsonObject>>(
          Summary.Regressions, regressionObject)));
  DataAdapters::WriteBooleanField(Object, TEXT("regressionGatePassed"),
                                  Summary.bRegressionGatePassed);
  DataAdapters::WriteBooleanField(Object, TEXT("qualityGatePassed"),
                                  Summary.bQualityGatePassed);
  return Object;
}

} // namespace QualityReportSerializationAdaptersDetail

/** User Story: As a release operator, I need a report converted into the same stable JSON contract used by the TypeScript micro-game. @fn inline TSharedRef<FJsonObject> qualityReportObject(const FQualityReport &Report) */
inline TSharedRef<FJsonObject>
qualityReportObject(const FQualityReport &Report) {
  using namespace QualityReportSerializationAdaptersDetail;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  DataAdapters::WriteNumberField(Object, TEXT("schemaVersion"),
                                 Report.SchemaVersion);
  DataAdapters::WriteStringField(Object, TEXT("contractVersion"),
                                 Report.ContractVersion);
  DataAdapters::WriteStringField(Object, TEXT("evaluationScope"),
                                 Report.EvaluationScope);
  DataAdapters::WriteStringField(Object, TEXT("host"), Report.Host);
  DataAdapters::WriteStringField(Object, TEXT("generatedAt"),
                                 Report.GeneratedAt);
  DataAdapters::WriteObjectField(Object, TEXT("metadata"),
                                 metadataObject(Report.Metadata));
  Report.Error.IsEmpty()
      ? DataAdapters::WriteNullField(Object, TEXT("error"))
      : DataAdapters::WriteStringField(Object, TEXT("error"), Report.Error);
  DataAdapters::WriteObjectField(Object, TEXT("summary"),
                                 summaryObject(Report.Summary));
  DataAdapters::WriteArrayField(
      Object, TEXT("samples"),
      objectValues(func::map_array<FQualitySample, TSharedRef<FJsonObject>>(
          Report.Samples, sampleObject)));
  return Object;
}

/** User Story: As a report writer, I need deterministic pretty JSON produced through Unreal's structured serializer. @fn inline FString serializeQualityReport(const FQualityReport &Report) */
inline FString serializeQualityReport(const FQualityReport &Report) {
  FString Json;
  const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
      TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Json);
  FJsonSerializer::Serialize(qualityReportObject(Report), Writer);
  return Json;
}

/** User Story: As a before-and-after evaluator, I need only comparison-authoritative baseline fields projected from persisted JSON. @fn inline FQualityReport readQualityBaselineReport(const TSharedRef<FJsonObject> &Object) */
inline FQualityReport
readQualityBaselineReport(const TSharedRef<FJsonObject> &Object) {
  const TSharedRef<FJsonObject> Metadata =
      DataAdapters::ReadObjectField(Object, TEXT("metadata"));
  const TSharedRef<FJsonObject> Summary =
      DataAdapters::ReadObjectField(Object, TEXT("summary"));
  const TSharedRef<FJsonObject> MetricsObject =
      DataAdapters::ReadObjectField(Summary, TEXT("metrics"));
  const TMap<FString, FQualityMetricSummary> Metrics =
      func::fold_array<FString, TMap<FString, FQualityMetricSummary>>(
          qualityData().MetricNames, {},
          [&MetricsObject](
              const TMap<FString, FQualityMetricSummary> &Acc,
              const FString &Metric) {
            const TSharedRef<FJsonObject> Value =
                DataAdapters::ReadObjectField(MetricsObject, Metric);
            TMap<FString, FQualityMetricSummary> Next = Acc;
            Next.Add(Metric,
                     {DataAdapters::ReadNumberField(Value, TEXT("passed")),
                      DataAdapters::ReadNumberField(Value, TEXT("total")),
                      DataAdapters::ReadDoubleField(Value, TEXT("passRate")),
                      DataAdapters::ReadBooleanField(Value,
                                                     TEXT("gatePassed")),
                      DataAdapters::ReadStringArrayField(
                          Value, TEXT("violations"))});
            return Next;
          });
  return {DataAdapters::ReadNumberField(Object, TEXT("schemaVersion")),
          DataAdapters::ReadStringField(Object, TEXT("contractVersion")),
          DataAdapters::ReadStringField(Object, TEXT("evaluationScope")),
          DataAdapters::ReadStringField(Object, TEXT("host")),
          DataAdapters::ReadStringField(Object, TEXT("generatedAt")),
          {DataAdapters::ReadStringField(Metadata, TEXT("apiStatus")),
           DataAdapters::ReadStringField(Metadata, TEXT("apiVersion")),
           DataAdapters::ReadNumberField(Metadata,
                                         TEXT("inferenceLatencyBudgetMs")),
           DataAdapters::ReadStringField(Metadata, TEXT("slmStatus")),
           DataAdapters::ReadStringField(Metadata, TEXT("slmVersion")),
           DataAdapters::ReadStringField(Metadata,
                                         TEXT("slotContractVersion")),
           DataAdapters::ReadStringField(Metadata,
                                         TEXT("slmArtifactSha256"))},
          DataAdapters::ReadOptionalStringField(Object, TEXT("error")),
          {Metrics,
           {},
           DataAdapters::ReadDoubleField(Summary, TEXT("meanLatencyMs")),
           DataAdapters::ReadDoubleField(Summary,
                                         TEXT("percentileLatencyMs")),
           DataAdapters::ReadBooleanField(Summary,
                                          TEXT("absoluteGatePassed")),
           DataAdapters::ReadStringField(Summary, TEXT("baselineStatus")),
           {},
           DataAdapters::ReadBooleanField(Summary,
                                          TEXT("regressionGatePassed")),
           DataAdapters::ReadBooleanField(Summary,
                                          TEXT("qualityGatePassed"))},
          func::map_array<TSharedPtr<FJsonValue>, FQualitySample>(
              DataAdapters::ReadObjectArrayField(Object, TEXT("samples")),
              QualityReportSerializationAdaptersDetail::baselineSample)};
}

} // namespace MicroGame
