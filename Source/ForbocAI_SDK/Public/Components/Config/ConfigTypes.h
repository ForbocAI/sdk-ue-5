#pragma once

#include "CoreMinimal.h"

namespace ConfigSlice {

using FConfigEntries = TMap<FString, FString>;

struct FConfigDefaultsData {
  FString SdkVersion;
  FString ApiUrl;
  FString ApiKey;
  FString DatabasePath;
  int32 VectorDimension;
  int32 MaxRecallResults;
};

struct FConfigFieldData {
  FString SdkVersion;
  FString ApiUrl;
  FString ApiKey;
  FString DatabasePath;
  FString VectorDimension;
  FString MaxRecallResults;
};

struct FConfigActionData {
  FString Hydrated;
  FString EntryCommitted;
  FString ApiCommitted;
};

struct FConfigSliceData {
  FString Name;
  FString ActionSeparator;
  FConfigActionData Actions;
};

struct FConfigEnvironmentData {
  FString HomeKey;
  FString ProfileKey;
  FString ApiUrlKey;
  FString ApiKeyKey;
  FString DatabasePathKey;
  FString VectorDimensionKey;
  FString MaxRecallResultsKey;
};

struct FConfigPathData {
  FString HomeFallback;
  FString FileName;
};

struct FConfigSerializationData {
  FString Encoding;
  int32 Indentation;
  int32 FileMode;
};

struct FConfigErrorData {
  FString InvalidRoot;
};

struct FConfigRuntimeData {
  FConfigDefaultsData Defaults;
  FConfigFieldData Fields;
  FConfigSliceData Slice;
  FConfigEnvironmentData Environment;
  FConfigPathData Path;
  FConfigSerializationData Serialization;
  FConfigErrorData Errors;
};

struct FConfigInvocationOverrides {
  FString ApiUrl;
  FString ApiKey;
};

struct FConfigEntryCommitted {
  FString Key;
  FString Value;
};

struct FConfigApiCommitted {
  FString ApiUrl;
  FString ApiKey;
};

struct FConfigState {
  FConfigEntries Entries;
  FConfigFieldData Fields;
  FString FilePath;
  FString SdkVersion;
  FString ApiUrl;
  FString ApiKey;
  FString DatabasePath;
  int32 VectorDimension;
  int32 MaxRecallResults;
};

} // namespace ConfigSlice
