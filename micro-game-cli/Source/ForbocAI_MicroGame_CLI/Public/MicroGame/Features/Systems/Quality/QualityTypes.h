#pragma once

#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "MicroGame/Features/Systems/Quality/Vocabulary/QualityVocabularyTypes.h"

namespace MicroGame {

struct FQualityModelMetadata {
  FString ApiStatus;
  FString ApiVersion;
  int32 InferenceLatencyBudgetMs{};
  FString SlmStatus;
  FString SlmVersion;
  FString SlotContractVersion;
  FString SlmArtifactSha256;
};

struct FQualityMetricEvaluation {
  bool bPassed{};
  double Score{};
  TArray<FString> Violations;
};

struct FQualitySample {
  FString Id;
  FString Category;
  FString PairKey;
  FString Command;
  FString Response;
  FString Reference;
  double ReferenceTokenF1{};
  double RepeatedNgramRatio{};
  double DurationMs{};
  TMap<FString, FQualityMetricEvaluation> Metrics;
};

struct FQualityPairComparison {
  FString PairKey;
  TArray<FString> SampleIds;
  double TokenJaccard{};
  bool bPassed{};
  TArray<FString> Violations;
};

struct FQualityMetricSummary {
  int32 Passed{};
  int32 Total{};
  double PassRate{};
  bool bGatePassed{};
  TArray<FString> Violations;
};

struct FQualityRegression {
  FString Metric;
  FString Measure;
  double BaselineValue{};
  double CurrentValue{};
  bool bRegressed{};
};

struct FQualityReportSummary {
  TMap<FString, FQualityMetricSummary> Metrics;
  TArray<FQualityPairComparison> PairComparisons;
  double MeanLatencyMs{};
  double PercentileLatencyMs{};
  bool bAbsoluteGatePassed{};
  FString BaselineStatus;
  TArray<FQualityRegression> Regressions;
  bool bRegressionGatePassed{};
  bool bQualityGatePassed{};
};

struct FQualityReport {
  int32 SchemaVersion{};
  FString ContractVersion;
  FString EvaluationScope;
  FString Host;
  FString GeneratedAt;
  FQualityModelMetadata Metadata;
  FString Error;
  FQualityReportSummary Summary;
  TArray<FQualitySample> Samples;
};

struct FQualityState {
  bool bRequired{};
  FString Status;
  FString Host;
  FString StartedAt;
  FString CompletedAt;
  func::Maybe<FQualityModelMetadata> Metadata;
  func::Maybe<FQualityReport> Baseline;
  FString Error;
  TMap<FString, FQualitySample> Samples;
};

struct FQualityEvaluationStartedPayload {
  FString Host;
  FString StartedAt;
  func::Maybe<FQualityReport> Baseline;
};

struct FQualityEvaluationCompletedPayload {
  FString CompletedAt;
};

struct FQualityEvaluationFailedPayload {
  FString CompletedAt;
  FString Error;
};

struct FQualitySetupFailure {
  FCommandSpec Command;
  CommandRunner::FCommandOutput Result;
};

} // namespace MicroGame
