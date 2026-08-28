#pragma once

#include "Components/Ghost/Results/ResultsTypes.h"
#include "Systems/API/Endpoints/Ghost/Identity/GhostIdentityAdapters.h"

namespace APISlice::Detail {

/**
 * User Story: As Ghost diagnostics, I need every API evidence counter retained as one atomic summary.
 * @fn inline func::Maybe<FGhostEvidenceSummary> DecodeGhostEvidenceObject( const TSharedPtr<FJsonObject> &Object)
 */
inline func::Maybe<FGhostEvidenceSummary> DecodeGhostEvidenceObject(
    const TSharedPtr<FJsonObject> &Object) {
  const auto &Fields =
      Endpoints::GhostConfiguration::ghostApiConfiguration().Fields.Evidence;
  FGhostEvidenceSummary Value;
  const bool bValid =
      DecodeGhostInt32Field(Object, Fields.CompletedProcessTurns,
                            Value.CompletedProcessTurns) &&
      DecodeGhostInt32Field(Object, Fields.ValidCognitionOutcomes,
                            Value.ValidCognitionOutcomes) &&
      DecodeGhostInt32Field(Object, Fields.InvalidCognitionOutcomes,
                            Value.InvalidCognitionOutcomes) &&
      DecodeGhostInt32Field(Object, Fields.SignedCognitionOutcomes,
                            Value.SignedCognitionOutcomes) &&
      DecodeGhostInt32Field(Object, Fields.ThoughtResults,
                            Value.ThoughtResults) &&
      DecodeGhostInt32Field(Object, Fields.ReasoningResults,
                            Value.ReasoningResults) &&
      DecodeGhostInt32Field(Object, Fields.ConsistentReasoningResults,
                            Value.ConsistentReasoningResults) &&
      DecodeGhostInt32Field(Object, Fields.DiagnosticResults,
                            Value.DiagnosticResults) &&
      DecodeGhostInt32Field(Object, Fields.GroundedDiagnosticResults,
                            Value.GroundedDiagnosticResults) &&
      DecodeGhostInt32Field(Object, Fields.Contradictions,
                            Value.Contradictions) &&
      DecodeGhostInt32Field(Object, Fields.UnsupportedClaims,
                            Value.UnsupportedClaims) &&
      DecodeGhostInt32Field(Object, Fields.Failures, Value.Failures) &&
      DecodeGhostInt32Field(Object, Fields.Timeouts, Value.Timeouts) &&
      DecodeGhostInt32Field(Object, Fields.UniqueObservations,
                            Value.UniqueObservations) &&
      DecodeGhostInt32Field(Object, Fields.ChoicePoints, Value.ChoicePoints);
  return bValid ? func::just(Value)
                : func::nothing<FGhostEvidenceSummary>();
}

/**
 * User Story: As Ghost result decoding, I need each check preserve required fields and validate nullable attachments.
 * @fn inline func::Maybe<FGhostResultRecord> DecodeGhostTestObject( const TSharedPtr<FJsonObject> &Object)
 */
inline func::Maybe<FGhostResultRecord> DecodeGhostTestObject(
    const TSharedPtr<FJsonObject> &Object) {
  const auto &Fields =
      Endpoints::GhostConfiguration::ghostApiConfiguration().Fields.Test;
  FGhostResultRecord Record;
  const bool bValid =
      DecodeGhostStringField(Object, Fields.Name, Record.TestName) &&
      Object.IsValid() && Object->TryGetBoolField(Fields.Passed,
                                                 Record.bTestPassed) &&
      DecodeGhostInt64Field(Object, Fields.Duration, Record.TestDuration) &&
      DecodeGhostOptionalStringField(Object, Fields.Error,
                                     Record.TestError) &&
      DecodeGhostOptionalStringField(Object, Fields.Screenshot,
                                     Record.TestScreenshot);
  return bValid ? func::just(Record)
                : func::nothing<FGhostResultRecord>();
}

/**
 * User Story: As strict Ghost check traversal, I need non-object entries reject the complete results response.
 * @fn inline func::Maybe<FGhostResultRecord> DecodeGhostTestValue( const TSharedPtr<FJsonValue> &Value)
 */
inline func::Maybe<FGhostResultRecord> DecodeGhostTestValue(
    const TSharedPtr<FJsonValue> &Value) {
  return Value.IsValid() && Value->Type == EJson::Object
             ? DecodeGhostTestObject(Value->AsObject())
             : func::nothing<FGhostResultRecord>();
}

/**
 * User Story: As Ghost metric decoding, I need each tuple validated against the authored pair shape before entering SDK state.
 * @fn inline func::Maybe<TPair<FString, float>> DecodeGhostMetricValue( const TSharedPtr<FJsonValue> &Value)
 */
inline func::Maybe<TPair<FString, float>> DecodeGhostMetricValue(
    const TSharedPtr<FJsonValue> &Value) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const TArray<TSharedPtr<FJsonValue>> *Pair =
      Value.IsValid() && Value->Type == EJson::Array
          ? &Value->AsArray()
          : nullptr;
  const bool bValid =
      Pair && Pair->Num() == Data.Limits.MetricPairSize &&
      Pair->IsValidIndex(Data.Limits.MetricNameIndex) &&
      Pair->IsValidIndex(Data.Limits.MetricValueIndex) &&
      (*Pair)[Data.Limits.MetricNameIndex].IsValid() &&
      (*Pair)[Data.Limits.MetricNameIndex]->Type == EJson::String &&
      (*Pair)[Data.Limits.MetricValueIndex].IsValid() &&
      (*Pair)[Data.Limits.MetricValueIndex]->Type == EJson::Number &&
      FMath::IsFinite(
          (*Pair)[Data.Limits.MetricValueIndex]->AsNumber());
  return !bValid
             ? func::nothing<TPair<FString, float>>()
             : func::just(TPair<FString, float>(
                   (*Pair)[Data.Limits.MetricNameIndex]->AsString(),
                   static_cast<float>(
                       (*Pair)[Data.Limits.MetricValueIndex]->AsNumber())));
}

/**
 * User Story: As Ghost metrics state, I need a fully validated tuple collection folded into one named map.
 * @fn inline func::Maybe<TMap<FString, float>> DecodeGhostMetrics( const TArray<TSharedPtr<FJsonValue>> &Values)
 */
inline func::Maybe<TMap<FString, float>> DecodeGhostMetrics(
    const TArray<TSharedPtr<FJsonValue>> &Values) {
  const func::Maybe<TArray<TPair<FString, float>>> Pairs =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                 TPair<FString, float>>(
          Values, DecodeGhostMetricValue);
  return func::is_nothing(Pairs)
             ? func::nothing<TMap<FString, float>>()
             : func::just(func::fold_array<TPair<FString, float>,
                                           TMap<FString, float>>(
                   Pairs.value, TMap<FString, float>(),
                   [](const TMap<FString, float> &Metrics,
                      const TPair<FString, float> &Metric) {
                     return func::upsert_map_value<FString, float>(
                         Metrics, Metric.Key, Metric.Value,
                         [&Metric](float) { return Metric.Value; });
                   }));
}

/** User Story: As Ghost result consumers, I need API verdict, evidence, dimensions, checks, metrics, and summary decoded without client analysis. @fn inline bool DecodeGhostResultsResponse(const FString &Json, FGhostResults &Response) */
inline bool DecodeGhostResultsResponse(const FString &Json,
                                       FGhostResults &Response) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.Results;
  TSharedPtr<FJsonObject> Root;
  const bool bRootValid = JsonInterop::ParseJsonObject(Json, Root) &&
                          Root.IsValid();
  const TArray<TSharedPtr<FJsonValue>> *Tests = nullptr;
  const TArray<TSharedPtr<FJsonValue>> *Metrics = nullptr;
  const bool bArraysValid =
      bRootValid && Root->TryGetArrayField(Fields.Tests, Tests) && Tests &&
      Root->TryGetArrayField(Fields.Metrics, Metrics) && Metrics;
  const func::Maybe<FGhostRuntimeIdentity> RuntimeIdentity =
      bRootValid ? DecodeGhostRuntimeIdentityField(Root, Fields.RuntimeIdentity)
                 : func::nothing<FGhostRuntimeIdentity>();
  const func::Maybe<FGhostEvidenceSummary> Evidence =
      bRootValid && Root->HasTypedField<EJson::Object>(Fields.Evidence)
          ? DecodeGhostEvidenceObject(Root->GetObjectField(Fields.Evidence))
          : func::nothing<FGhostEvidenceSummary>();
  const func::Maybe<TArray<FString>> EvidenceDimensions =
      bRootValid ? DecodeGhostStringArrayField(Root, Fields.EvidenceDimensions)
                 : func::nothing<TArray<FString>>();
  const func::Maybe<TArray<FString>> EvaluationDimensions =
      bRootValid
          ? DecodeGhostStringArrayField(Root, Fields.EvaluationDimensions)
          : func::nothing<TArray<FString>>();
  const func::Maybe<TArray<FGhostResultRecord>> DecodedTests =
      bArraysValid
          ? func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                       FGhostResultRecord>(
                *Tests, DecodeGhostTestValue)
          : func::nothing<TArray<FGhostResultRecord>>();
  const func::Maybe<TMap<FString, float>> DecodedMetrics =
      bArraysValid ? DecodeGhostMetrics(*Metrics)
                   : func::nothing<TMap<FString, float>>();

  FGhostResults Decoded;
  const bool bValid =
      bRootValid && bArraysValid && !func::is_nothing(RuntimeIdentity) &&
      !func::is_nothing(Evidence) &&
      !func::is_nothing(EvidenceDimensions) &&
      !func::is_nothing(EvaluationDimensions) &&
      !func::is_nothing(DecodedTests) && !func::is_nothing(DecodedMetrics) &&
      DecodeGhostStringField(Root, Fields.SessionId, Decoded.SessionId) &&
      DecodeGhostIdentityField(Root, Fields.GhostName,
                               Data.Limits.MinimumIdentityLength,
                               Decoded.GhostName) &&
      DecodeGhostInt32Field(Root, Fields.TotalTests, Decoded.TotalTests) &&
      DecodeGhostInt32Field(Root, Fields.Passed, Decoded.Passed) &&
      DecodeGhostInt32Field(Root, Fields.Failed, Decoded.Failed) &&
      DecodeGhostInt32Field(Root, Fields.Skipped, Decoded.Skipped) &&
      DecodeGhostInt64Field(Root, Fields.Duration, Decoded.Duration) &&
      DecodeGhostFloatField(Root, Fields.Coverage, Decoded.Coverage) &&
      DecodeGhostStringField(Root, Fields.Verdict, Decoded.Verdict) &&
      DecodeGhostStringField(Root, Fields.Summary, Decoded.Summary);
  return !bValid
             ? false
             : (Decoded.RuntimeIdentity = RuntimeIdentity.value,
                Decoded.Evidence = Evidence.value,
                Decoded.EvidenceDimensions = EvidenceDimensions.value,
                Decoded.EvaluationDimensions = EvaluationDimensions.value,
                Decoded.Tests = DecodedTests.value,
                Decoded.Metrics = DecodedMetrics.value,
                Response = Decoded, true);
}

} // namespace APISlice::Detail
