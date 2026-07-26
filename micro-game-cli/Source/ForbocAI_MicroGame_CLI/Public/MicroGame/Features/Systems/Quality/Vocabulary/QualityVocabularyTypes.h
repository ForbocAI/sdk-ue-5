#pragma once

#include "CoreMinimal.h"

namespace MicroGame {

struct FQualityRunStatuses {
  FString Idle;
  FString Running;
  FString Completed;
  FString Failed;
};

struct FQualityCommandStatuses {
  FString Ok;
  FString Error;
};

struct FQualityBaselineStatuses {
  FString Missing;
  FString Compatible;
  FString Incompatible;
};

struct FQualityRegressionMeasures {
  FString PassRate;
  FString MeanLatencyMs;
};

struct FQualityMetrics {
  FString Coherence;
  FString InstructionFollowing;
  FString CharacterConsistency;
  FString Repetition;
  FString Latency;
};

struct FQualityCategories {
  FString Coherence;
  FString InstructionFollowing;
  FString CharacterConsistency;
};

struct FQualityNumbers {
  int32 EmptyCount{};
  int32 InitialIndex{};
  int32 NextIndex{};
  int32 SingularCount{};
  int32 PercentageScale{};
  int32 RoundingPrecision{};
  int32 RoundingBase{};
  int32 MinimumResponseWords{};
  int32 MaximumResponseWords{};
  int32 RepetitionNgramSize{};
  double MaximumRepeatedNgramRatio{};
  double MinimumPairTokenJaccard{};
  double MinimumReferenceF1{};
  int32 FallbackInferenceLatencyBudgetMs{};
  double LatencyPercentile{};
  double RateRegressionTolerance{};
  double LatencyRegressionToleranceRatio{};
};

struct FQualityPatterns {
  FString Words;
  FString Whitespace;
  FString TrailingPunctuation;
  FString Ansi;
  FString CaseInsensitivePrefix;
  TArray<FString> SystemLeaks;
  TArray<FString> RoleEscapes;
  TArray<FString> SpecificSelfClaims;
  TArray<FString> IntrinsicIdentityClaims;
  TArray<FString> UnknownBoundary;
};

struct FQualityOutput {
  FString LineSeparator;
  FString Empty;
  FString Space;
  FString ViolationSeparator;
  FString ObjectStart;
  FString ObjectEnd;
  TArray<FString> DialoguePrefixes;
  TArray<FString> IgnoredPrefixes;
};

struct FQualityMetadataFields {
  FString ApiStatus;
  FString ApiVersion;
  FString InferenceLatencyBudgetMs;
  FString SlmStatus;
  FString SlmVersion;
  FString SlotContractVersion;
  FString SlmArtifactSha256;
};

struct FQualityMessages {
  FString SetupFailed;
  FString MetadataFailed;
  FString ProbeFailed;
  FString DialogueMissing;
  FString CommandFailed;
  FString ResponseTooShort;
  FString ResponseTooLong;
  FString SystemPromptLeakage;
  FString RoleEscape;
  FString ExactResponseMismatch;
  FString RequiredTokenGroupMissing;
  FString RequiredPatternMissing;
  FString ForbiddenPatternMatched;
  FString ReferenceF1BelowMinimum;
  FString SpecificSelfClaim;
  FString IntrinsicIdentityClaim;
  FString UnknownBoundaryOmitted;
  FString RepeatedNgramRatioExceeded;
  FString LatencyMissing;
  FString LatencyBudgetExceeded;
  FString PairSimilarityBelowMinimum;
  FString BaselineMissing;
  FString BaselineIncompatible;
  FString AbsoluteGateFailed;
  FString RegressionGateFailed;
  FString QualityPassed;
  FString QualityNotRequired;
  FString ReportHeading;
  FString ReportMetric;
  FString ReportModel;
  FString ReportBaseline;
  FString ReportGate;
  FString ReportPath;
  FString Passed;
  FString Failed;
};

struct FQualityTokens {
  FString Command;
  FString Output;
  FString Id;
  FString Metric;
  FString Passed;
  FString Total;
  FString Rate;
  FString Version;
  FString Artifact;
  FString ApiVersion;
  FString Status;
  FString Path;
};

struct FQualityHostData {
  double OverheadBudgetMs{};
};

struct FQualityCommandData {
  FString Group;
  FString Command;
  TArray<FString> ExpectedRoutes;
};

struct FQualityProbe {
  FString Id;
  FString Category;
  FString PairKey;
  FString Command;
  FString Reference;
  FString ExactResponse;
  TArray<TArray<FString>> RequiredTokenGroups;
  TArray<FString> RequiredPatterns;
  TArray<FString> ForbiddenPatterns;
  double MinimumReferenceF1{};
  int32 MaximumWords{};
  bool bCoherenceUsesExpectations{};
  bool bRequiresUnknownBoundary{};
};

struct FQualityData {
  int32 SchemaVersion{};
  FString ContractVersion;
  FString EvaluationScope;
  FQualityRunStatuses RunStatuses;
  FQualityCommandStatuses CommandStatuses;
  FQualityBaselineStatuses BaselineStatuses;
  FQualityRegressionMeasures RegressionMeasures;
  TMap<FString, FQualityHostData> Hosts;
  FQualityMetrics Metrics;
  TArray<FString> MetricNames;
  FQualityCategories Categories;
  FQualityNumbers Numbers;
  FQualityPatterns Patterns;
  FQualityOutput Output;
  FQualityMetadataFields MetadataFields;
  FQualityMessages Messages;
  FQualityTokens Tokens;
  FQualityCommandData MetadataCommand;
  TArray<FQualityCommandData> SetupCommands;
  FQualityCommandData ProbeCommand;
  TArray<FQualityProbe> Probes;
};

} // namespace MicroGame
