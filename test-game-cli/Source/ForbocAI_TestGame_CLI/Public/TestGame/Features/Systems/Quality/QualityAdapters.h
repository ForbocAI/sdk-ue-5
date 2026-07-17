#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Quality/QualityTypes.h"
#include "TestGame/Features/Systems/Quality/Vocabulary/QualityVocabularyAdapters.h"

namespace TestGame {

/** User Story: As a features systems quality consumer, I need to invoke quality data through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline const FQualityData &qualityData() */
inline const FQualityData &qualityData();

/** User Story: As a quality evaluator, I need deterministic report precision shared by scoring and selectors. @fn inline double roundQualityNumber(double Value) */
inline double roundQualityNumber(double Value) {
  const double Scale = FMath::Pow(
      static_cast<double>(qualityData().Numbers.RoundingBase),
      static_cast<double>(qualityData().Numbers.RoundingPrecision));
  return FMath::RoundToDouble(Value * Scale) / Scale;
}

/** User Story: As a quality evaluator, I need duplicate violation evidence collapsed without discarding distinct failures. @fn inline TArray<FString> uniqueQualityValues(const TArray<FString> &Values) */
inline TArray<FString>
uniqueQualityValues(const TArray<FString> &Values) {
  TArray<FString> Result = func::fold_array<FString, TArray<FString>>(
      Values, {}, [](const TArray<FString> &Unique, const FString &Value) {
        return Unique.Contains(Value)
                   ? Unique
                   : func::append_value<FString>(Unique, Value);
      });
  Result.Sort();
  return Result;
}

/**
 * User Story: As a quality presenter, I need authored templates populated without presentation strings in source.
 * @fn inline FString formatQualityTemplate( FString Template, const TMap<FString, FString> &Values)
 */
inline FString formatQualityTemplate(
    FString Template, const TMap<FString, FString> &Values) {
  TArray<FString> Keys;
  Values.GetKeys(Keys);
  return func::fold_array<FString, FString>(
      Keys, MoveTemp(Template), [&Values](const FString &Result,
                                          const FString &Token) {
        return Result.Replace(*Token, *Values[Token]);
      });
}

/** User Story: As TS and UE quality evaluators, I need one typed view of the byte-identical authored quality contract. @fn inline const FQualityData &qualityData() */
inline const FQualityData &qualityData() {
  static const FQualityData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("systems/quality.json"));
    const DataAdapters::FArraySource ProbeSource =
        DataAdapters::ArraySource(TEXT("systems/quality/probes.json"));
    const TSharedRef<FJsonObject> Root = Source.Root;
    const TSharedRef<FJsonObject> Run =
        DataAdapters::ReadObjectField(Root, TEXT("runStatuses"));
    const TSharedRef<FJsonObject> Command =
        DataAdapters::ReadObjectField(Root, TEXT("commandStatuses"));
    const TSharedRef<FJsonObject> Baseline =
        DataAdapters::ReadObjectField(Root, TEXT("baselineStatuses"));
    const TSharedRef<FJsonObject> Regression =
        DataAdapters::ReadObjectField(Root, TEXT("regressionMeasures"));
    const TSharedRef<FJsonObject> Hosts =
        DataAdapters::ReadObjectField(Root, TEXT("hosts"));
    const TSharedRef<FJsonObject> Metrics =
        DataAdapters::ReadObjectField(Root, TEXT("metrics"));
    const TSharedRef<FJsonObject> Categories =
        DataAdapters::ReadObjectField(Root, TEXT("categories"));
    const TSharedRef<FJsonObject> Numbers =
        DataAdapters::ReadObjectField(Root, TEXT("numbers"));
    const TSharedRef<FJsonObject> Flags =
        DataAdapters::ReadObjectField(Root, TEXT("flags"));
    const TSharedRef<FJsonObject> Patterns =
        DataAdapters::ReadObjectField(Root, TEXT("patterns"));
    const TSharedRef<FJsonObject> Output =
        DataAdapters::ReadObjectField(Root, TEXT("output"));
    const TSharedRef<FJsonObject> Metadata =
        DataAdapters::ReadObjectField(Root, TEXT("metadataFields"));
    const TSharedRef<FJsonObject> Messages =
        DataAdapters::ReadObjectField(Root, TEXT("messages"));
    const TSharedRef<FJsonObject> Tokens =
        DataAdapters::ReadObjectField(Root, TEXT("tokens"));
    const TSharedRef<FJsonObject> MetadataCommand =
        DataAdapters::ReadObjectField(Root, TEXT("metadataCommand"));
    const TSharedRef<FJsonObject> ProbeCommand =
        DataAdapters::ReadObjectField(Root, TEXT("probeCommand"));
    return FQualityData{
        DataAdapters::ReadNumberField(Root, TEXT("schemaVersion")),
        DataAdapters::ReadStringField(Root, TEXT("contractVersion")),
        {DataAdapters::ReadStringField(Run, TEXT("idle")),
         DataAdapters::ReadStringField(Run, TEXT("running")),
         DataAdapters::ReadStringField(Run, TEXT("completed")),
         DataAdapters::ReadStringField(Run, TEXT("failed"))},
        {DataAdapters::ReadStringField(Command, TEXT("ok")),
         DataAdapters::ReadStringField(Command, TEXT("error"))},
        {DataAdapters::ReadStringField(Baseline, TEXT("missing")),
         DataAdapters::ReadStringField(Baseline, TEXT("compatible")),
         DataAdapters::ReadStringField(Baseline, TEXT("incompatible"))},
        {DataAdapters::ReadStringField(Regression, TEXT("passRate")),
         DataAdapters::ReadStringField(Regression, TEXT("meanLatencyMs"))},
        QualityVocabularyAdapters::ReadHosts(Hosts),
        {DataAdapters::ReadStringField(Metrics, TEXT("coherence")),
         DataAdapters::ReadStringField(Metrics,
                                       TEXT("instructionFollowing")),
         DataAdapters::ReadStringField(Metrics,
                                       TEXT("characterConsistency")),
         DataAdapters::ReadStringField(Metrics, TEXT("repetition")),
         DataAdapters::ReadStringField(Metrics, TEXT("latency"))},
        QualityVocabularyAdapters::ObjectKeys(Metrics),
        {DataAdapters::ReadStringField(Categories, TEXT("coherence")),
         DataAdapters::ReadStringField(Categories,
                                       TEXT("instructionFollowing")),
         DataAdapters::ReadStringField(Categories,
                                       TEXT("characterConsistency"))},
        {DataAdapters::ReadNumberField(Numbers, TEXT("emptyCount")),
         DataAdapters::ReadNumberField(Numbers, TEXT("initialIndex")),
         DataAdapters::ReadNumberField(Numbers, TEXT("nextIndex")),
         DataAdapters::ReadNumberField(Numbers, TEXT("singularCount")),
         DataAdapters::ReadNumberField(Numbers, TEXT("percentageScale")),
         DataAdapters::ReadNumberField(Numbers, TEXT("roundingPrecision")),
         DataAdapters::ReadNumberField(Numbers, TEXT("roundingBase")),
         DataAdapters::ReadNumberField(Numbers, TEXT("minimumResponseWords")),
         DataAdapters::ReadNumberField(Numbers, TEXT("maximumResponseWords")),
         DataAdapters::ReadNumberField(Numbers, TEXT("repetitionNgramSize")),
         DataAdapters::ReadFloatField(Numbers,
                                      TEXT("maximumRepeatedNgramRatio")),
         DataAdapters::ReadFloatField(Numbers,
                                      TEXT("minimumPairTokenJaccard")),
         DataAdapters::ReadFloatField(Numbers, TEXT("minimumReferenceF1")),
         DataAdapters::ReadNumberField(
             Numbers, TEXT("fallbackInferenceLatencyBudgetMs")),
         DataAdapters::ReadFloatField(Numbers, TEXT("latencyPercentile")),
         DataAdapters::ReadFloatField(Numbers,
                                      TEXT("rateRegressionTolerance")),
         DataAdapters::ReadFloatField(
             Numbers, TEXT("latencyRegressionToleranceRatio"))},
        {DataAdapters::ReadStringField(Patterns, TEXT("words")),
         DataAdapters::ReadStringField(Patterns, TEXT("whitespace")),
         DataAdapters::ReadStringField(Patterns,
                                       TEXT("trailingPunctuation")),
         DataAdapters::ReadStringField(Patterns, TEXT("ansi")),
         DataAdapters::ReadStringField(Flags,
                                       TEXT("caseInsensitivePrefix")),
         DataAdapters::ReadStringArrayField(Patterns, TEXT("systemLeaks")),
         DataAdapters::ReadStringArrayField(Patterns, TEXT("roleEscapes")),
         DataAdapters::ReadStringArrayField(Patterns,
                                            TEXT("specificSelfClaims")),
         DataAdapters::ReadStringArrayField(
             Patterns, TEXT("intrinsicIdentityClaims")),
         DataAdapters::ReadStringArrayField(Patterns,
                                            TEXT("unknownBoundary"))},
        {DataAdapters::ReadStringField(Output, TEXT("lineSeparator")),
         DataAdapters::ReadStringField(Output, TEXT("empty")),
         DataAdapters::ReadStringField(Output, TEXT("space")),
         DataAdapters::ReadStringField(Output, TEXT("violationSeparator")),
         DataAdapters::ReadStringField(Output, TEXT("objectStart")),
         DataAdapters::ReadStringField(Output, TEXT("objectEnd")),
         DataAdapters::ReadStringArrayField(Output, TEXT("dialoguePrefixes")),
         DataAdapters::ReadStringArrayField(Output, TEXT("ignoredPrefixes"))},
        {DataAdapters::ReadStringField(Metadata, TEXT("apiStatus")),
         DataAdapters::ReadStringField(Metadata, TEXT("apiVersion")),
         DataAdapters::ReadStringField(
             Metadata, TEXT("inferenceLatencyBudgetMs")),
         DataAdapters::ReadStringField(Metadata, TEXT("slmStatus")),
         DataAdapters::ReadStringField(Metadata, TEXT("slmVersion")),
         DataAdapters::ReadStringField(Metadata,
                                       TEXT("slotContractVersion")),
         DataAdapters::ReadStringField(Metadata,
                                       TEXT("slmArtifactSha256"))},
        {DataAdapters::ReadStringField(Messages, TEXT("setupFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("metadataFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("probeFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("dialogueMissing")),
         DataAdapters::ReadStringField(Messages, TEXT("commandFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("responseTooShort")),
         DataAdapters::ReadStringField(Messages, TEXT("responseTooLong")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("systemPromptLeakage")),
         DataAdapters::ReadStringField(Messages, TEXT("roleEscape")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("exactResponseMismatch")),
         DataAdapters::ReadStringField(
             Messages, TEXT("requiredTokenGroupMissing")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("requiredPatternMissing")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("forbiddenPatternMatched")),
         DataAdapters::ReadStringField(
             Messages, TEXT("referenceF1BelowMinimum")),
         DataAdapters::ReadStringField(Messages, TEXT("specificSelfClaim")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("intrinsicIdentityClaim")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("unknownBoundaryOmitted")),
         DataAdapters::ReadStringField(
             Messages, TEXT("repeatedNgramRatioExceeded")),
         DataAdapters::ReadStringField(Messages, TEXT("latencyMissing")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("latencyBudgetExceeded")),
         DataAdapters::ReadStringField(
             Messages, TEXT("pairSimilarityBelowMinimum")),
         DataAdapters::ReadStringField(Messages, TEXT("baselineMissing")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("baselineIncompatible")),
         DataAdapters::ReadStringField(Messages, TEXT("absoluteGateFailed")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("regressionGateFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("qualityPassed")),
         DataAdapters::ReadStringField(Messages, TEXT("qualityNotRequired")),
         DataAdapters::ReadStringField(Messages, TEXT("reportHeading")),
         DataAdapters::ReadStringField(Messages, TEXT("reportMetric")),
         DataAdapters::ReadStringField(Messages, TEXT("reportModel")),
         DataAdapters::ReadStringField(Messages, TEXT("reportBaseline")),
         DataAdapters::ReadStringField(Messages, TEXT("reportGate")),
         DataAdapters::ReadStringField(Messages, TEXT("reportPath")),
         DataAdapters::ReadStringField(Messages, TEXT("passed")),
         DataAdapters::ReadStringField(Messages, TEXT("failed"))},
        {DataAdapters::ReadStringField(Tokens, TEXT("command")),
         DataAdapters::ReadStringField(Tokens, TEXT("output")),
         DataAdapters::ReadStringField(Tokens, TEXT("id")),
         DataAdapters::ReadStringField(Tokens, TEXT("metric")),
         DataAdapters::ReadStringField(Tokens, TEXT("passed")),
         DataAdapters::ReadStringField(Tokens, TEXT("total")),
         DataAdapters::ReadStringField(Tokens, TEXT("rate")),
         DataAdapters::ReadStringField(Tokens, TEXT("version")),
         DataAdapters::ReadStringField(Tokens, TEXT("artifact")),
         DataAdapters::ReadStringField(Tokens, TEXT("apiVersion")),
         DataAdapters::ReadStringField(Tokens, TEXT("status")),
         DataAdapters::ReadStringField(Tokens, TEXT("path"))},
        QualityVocabularyAdapters::ReadCommand(MetadataCommand),
        QualityVocabularyAdapters::ReadCommands(Root,
                                                 TEXT("setupCommands")),
        QualityVocabularyAdapters::ReadCommand(ProbeCommand),
        func::map_array<TSharedPtr<FJsonObject>, FQualityProbe>(
            DataAdapters::ReadObjectArray(ProbeSource),
            [](const TSharedPtr<FJsonObject> &Value) {
              return QualityVocabularyAdapters::ReadProbe(Value.ToSharedRef());
            })};
  }();
  return Data;
}

} // namespace TestGame
