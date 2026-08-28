#pragma once

#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Components/Ghost/Results/ResultsTypes.h"
#include "Dom/JsonObject.h"

namespace Testing::API::Codec::Ghost {

struct FGhostCodecLabels {
  FString Run;
  FString Status;
  FString Results;
  FString Stop;
  FString History;
  FString SessionId;
  FString GhostName;
  FString RuntimeIdentity;
  FString TestSuite;
  FString Progress;
  FString Evidence;
  FString EvidenceDimensions;
  FString EvaluationDimensions;
  FString TestCount;
  FString Coverage;
  FString Metric;
  FString Verdict;
  FString Summary;
  FString RejectsMissingIdentity;
  FString RejectsEmptyIdentity;
  FString RejectsMalformedTest;
  FString RejectsMalformedMetric;
  FString RejectsMalformedHistory;
};

struct FGhostCodecMalformedFixture {
  FString RunMissingIdentityJson;
  FString RunEmptyIdentityJson;
  int32 TestIndex;
  FString TestReplacement;
  int32 MetricIndex;
  FString MetricReplacement;
  int32 HistoryIndex;
  FString HistoryReplacement;
};

struct FGhostCodecProbeFixture {
  int32 EvidenceDimensionIndex;
  int32 EvaluationDimensionIndex;
  int32 TestIndex;
  int32 HistoryIndex;
  FString MetricName;
  float MetricValue;
};

struct FGhostCodecFixture {
  FString RunResponseJson;
  FGhostRunResponse RunExpected;
  FString StatusResponseJson;
  FGhostStatus StatusExpected;
  FString ResultsResponseJson;
  FGhostResults ResultsExpected;
  FString StopResponseJson;
  FGhostStopResponse StopExpected;
  FString HistoryResponseJson;
  FGhostHistoryResponse HistoryExpected;
  FGhostCodecMalformedFixture Malformed;
  FGhostCodecProbeFixture Probes;
  FGhostCodecLabels Labels;
};

} // namespace Testing::API::Codec::Ghost
