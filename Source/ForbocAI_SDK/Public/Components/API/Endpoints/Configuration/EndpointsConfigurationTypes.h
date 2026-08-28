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

struct FEndpointPayloadData {
  FString EmptyObject;
};

struct FEndpointTagIdData {
  FString List;
};

struct FEndpointDefaultData {
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
  FEndpointPayloadData Payloads;
  FEndpointTagIdData TagIds;
  FEndpointDefaultData Defaults;
  FEndpointTimeoutData Timeouts;
};

} // namespace APISlice::Endpoints::Configuration
