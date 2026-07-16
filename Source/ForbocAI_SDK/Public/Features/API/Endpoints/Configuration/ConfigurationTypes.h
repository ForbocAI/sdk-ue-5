#pragma once

#include "CoreMinimal.h"

namespace APISlice::Endpoints::Configuration {

struct FEndpointPathData {
  FString Separator;
  FString QueryStart;
  FString QueryAssignment;
};

struct FEndpointSegmentData {
  FString Bridge;
  FString Validate;
  FString Rules;
  FString Ghost;
  FString Run;
  FString Status;
  FString Results;
  FString Stop;
  FString History;
  FString Npcs;
  FString Process;
  FString Presets;
  FString Soul;
  FString Export;
  FString Confirm;
  FString Souls;
  FString Verify;
};

struct FEndpointQueryKeyData {
  FString Limit;
};

struct FEndpointNameData {
  FString GetApiStatus;
  FString PostNpcProcess;
  FString GetBridgeValidation;
  FString GetBridgeRules;
  FString PostGhostRun;
  FString GetGhostStatus;
  FString GetGhostResults;
  FString PostGhostStop;
  FString GetGhostHistory;
  FString PostBridgePreset;
  FString GetRulesets;
  FString GetRulePresets;
  FString PostSoulExportPreparation;
  FString PostSoulExportConfirmation;
  FString PostSoulVerification;
};

struct FEndpointFieldData {
  FString GhostRunSessionId;
  FString GhostRunStatus;
  FString GhostStatusSessionId;
  FString GhostStatusStatus;
  FString GhostStatusProgress;
  FString GhostStatusStartedAt;
  FString GhostStatusDuration;
  FString GhostStatusErrors;
  FString GhostResultsSessionId;
  FString GhostResultsTotalTests;
  FString GhostResultsPassed;
  FString GhostResultsFailed;
  FString GhostResultsSkipped;
  FString GhostResultsDuration;
  FString GhostResultsTests;
  FString GhostResultsCoverage;
  FString GhostResultsMetrics;
  FString GhostTestName;
  FString GhostTestPassed;
  FString GhostTestDuration;
  FString GhostTestError;
  FString GhostTestScreenshot;
  FString GhostStopStatus;
  FString GhostStopSessionId;
  FString GhostHistorySessions;
  FString GhostHistorySessionId;
  FString GhostHistoryTestSuite;
  FString GhostHistoryStartedAt;
  FString GhostHistoryCompletedAt;
  FString GhostHistoryStatus;
  FString GhostHistoryPassRate;
};

struct FEndpointValueData {
  FString Stopped;
};

struct FEndpointPayloadData {
  FString EmptyObject;
};

struct FEndpointTagIdData {
  FString List;
};

struct FEndpointDefaultData {
  int32 GhostHistoryLimit;
  int32 SoulListLimit;
};

struct FEndpointTimeoutData {
  int32 StatusMs;
};

struct FEndpointConfigurationData {
  FEndpointPathData Path;
  FEndpointSegmentData Segments;
  FEndpointQueryKeyData QueryKeys;
  FEndpointNameData Names;
  FEndpointFieldData Fields;
  FEndpointValueData Values;
  FEndpointPayloadData Payloads;
  FEndpointTagIdData TagIds;
  FEndpointDefaultData Defaults;
  FEndpointTimeoutData Timeouts;
};

} // namespace APISlice::Endpoints::Configuration
