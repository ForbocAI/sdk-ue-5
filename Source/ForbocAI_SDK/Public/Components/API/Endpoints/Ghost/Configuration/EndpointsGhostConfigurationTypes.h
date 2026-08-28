#pragma once

#include "CoreMinimal.h"

namespace APISlice::Endpoints::GhostConfiguration {

struct FGhostRunFieldData {
  FString SessionId;
  FString Status;
  FString GhostName;
  FString RuntimeIdentity;
};

struct FGhostRuntimeIdentityFieldData {
  FString ApiVersion;
  FString SlmStatus;
  FString SlmVersion;
  FString SlotContractVersion;
};

struct FGhostStatusFieldData {
  FString SessionId;
  FString GhostName;
  FString RuntimeIdentity;
  FString TestSuite;
  FString Status;
  FString Progress;
  FString StartedAt;
  FString Duration;
  FString Errors;
};

struct FGhostResultsFieldData {
  FString SessionId;
  FString GhostName;
  FString RuntimeIdentity;
  FString Evidence;
  FString EvidenceDimensions;
  FString EvaluationDimensions;
  FString TotalTests;
  FString Passed;
  FString Failed;
  FString Skipped;
  FString Duration;
  FString Tests;
  FString Coverage;
  FString Metrics;
  FString Verdict;
  FString Summary;
};

struct FGhostEvidenceFieldData {
  FString CompletedProcessTurns;
  FString ValidCognitionOutcomes;
  FString InvalidCognitionOutcomes;
  FString SignedCognitionOutcomes;
  FString ThoughtResults;
  FString ReasoningResults;
  FString ConsistentReasoningResults;
  FString DiagnosticResults;
  FString GroundedDiagnosticResults;
  FString Contradictions;
  FString UnsupportedClaims;
  FString Failures;
  FString Timeouts;
  FString UniqueObservations;
  FString ChoicePoints;
};

struct FGhostTestFieldData {
  FString Name;
  FString Passed;
  FString Duration;
  FString Error;
  FString Screenshot;
};

struct FGhostStopFieldData {
  FString Status;
  FString SessionId;
};

struct FGhostHistoryFieldData {
  FString Sessions;
  FString SessionId;
  FString GhostName;
  FString RuntimeIdentity;
  FString TestSuite;
  FString StartedAt;
  FString CompletedAt;
  FString Status;
  FString PassRate;
};

struct FGhostMetricFieldData {
  FString Name;
  FString Value;
};

struct FGhostFieldData {
  FGhostRunFieldData Run;
  FGhostRuntimeIdentityFieldData RuntimeIdentity;
  FGhostStatusFieldData Status;
  FGhostResultsFieldData Results;
  FGhostEvidenceFieldData Evidence;
  FGhostTestFieldData Test;
  FGhostStopFieldData Stop;
  FGhostHistoryFieldData History;
  FGhostMetricFieldData Metric;
};

struct FGhostValueData {
  FString Stopped;
};

struct FGhostLimitData {
  int32 MinimumIdentityLength;
  int32 MetricPairSize;
  int32 MetricNameIndex;
  int32 MetricValueIndex;
};

struct FGhostErrorData {
  FString Run;
  FString Identity;
  FString RuntimeIdentity;
  FString Status;
  FString Results;
  FString Evidence;
  FString Test;
  FString Stop;
  FString History;
  FString Metric;
};

struct FGhostApiConfigurationData {
  FGhostFieldData Fields;
  FGhostValueData Values;
  FGhostLimitData Limits;
  FGhostErrorData Errors;
};

} // namespace APISlice::Endpoints::GhostConfiguration
