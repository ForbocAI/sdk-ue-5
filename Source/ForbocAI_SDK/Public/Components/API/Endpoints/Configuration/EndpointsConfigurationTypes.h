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
  FString Generate;
  FString Presets;
  FString Soul;
  FString Export;
  FString Confirm;
  FString Souls;
  FString Verify;
  FString MicroGame;
  FString Contract;
};

struct FEndpointQueryKeyData {
  FString Limit;
};

struct FEndpointNameData {
  FString GetApiStatus;
  FString PostNpcProcess;
  FString PostNpcGenerateAttribute;
  FString GetBridgeValidation;
  FString GetBridgeRules;
  FString PostGhostRun;
  FString PostGhostProcess;
  FString PostGhostNpcGenerateAttribute;
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
  FString PostSoulStorageUpload;
  FString GetSoulStorageDownload;
  FString GetSoulStorageVerification;
  FString PostSoulStoragePreparation;
  FString DeleteSoulStoragePreparation;
  FString PostSoulStorageCommit;
  FString GetSoulStorageCatalog;
  FString GetSoulStorageEntry;
  FString GetMicroGameContract;
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

struct FEndpointTupleStructureData {
  int32 PairSize;
  int32 KeyIndex;
  int32 ValueIndex;
};

struct FEndpointStructureData {
  FEndpointTupleStructureData GhostMetric;
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
  int32 ContractMs;
  int32 NpcProcessMs;
  int32 NpcGenerateMs;
};

struct FEndpointConfigurationData {
  FEndpointPathData Path;
  FEndpointSegmentData Segments;
  FEndpointQueryKeyData QueryKeys;
  FEndpointNameData Names;
  FEndpointFieldData Fields;
  FEndpointStructureData Structures;
  FEndpointValueData Values;
  FEndpointPayloadData Payloads;
  FEndpointTagIdData TagIds;
  FEndpointDefaultData Defaults;
  FEndpointTimeoutData Timeouts;
};

} // namespace APISlice::Endpoints::Configuration
