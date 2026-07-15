#pragma once

#include "CoreMinimal.h"

namespace SDKConfig {

inline constexpr TCHAR DEFAULT_API_URL[] = TEXT("https://api.forboc.ai");
inline constexpr int32 DEFAULT_VECTOR_DIMENSION = 384;
inline constexpr int32 DEFAULT_MAX_RECALL_RESULTS = 10;

inline FString &ApiUrlStorage() {
  static FString Value = DEFAULT_API_URL;
  return Value;
}

inline FString &ApiKeyStorage() {
  static FString Value = TEXT("");
  return Value;
}

inline FString &DatabasePathStorage() {
  static FString Value = TEXT("");
  return Value;
}

inline int32 &VectorDimensionStorage() {
  static int32 Value = DEFAULT_VECTOR_DIMENSION;
  return Value;
}

inline int32 &MaxRecallResultsStorage() {
  static int32 Value = DEFAULT_MAX_RECALL_RESULTS;
  return Value;
}

inline bool &InitializedStorage() {
  static bool bInitialized = false;
  return bInitialized;
}

inline FString &ConfigFilePathOverrideStorage() {
  static FString Value;
  return Value;
}

inline void ResetToDefaults() {
  ApiUrlStorage() = DEFAULT_API_URL;
  ApiKeyStorage() = TEXT("");
  DatabasePathStorage() = TEXT("");
  VectorDimensionStorage() = DEFAULT_VECTOR_DIMENSION;
  MaxRecallResultsStorage() = DEFAULT_MAX_RECALL_RESULTS;
}

inline void InitializeConfig();
inline void EnsureInitialized();
inline void ReloadConfig();

inline FString GetApiUrl() {
  EnsureInitialized();
  return ApiUrlStorage();
}

inline FString GetApiKey() {
  EnsureInitialized();
  return ApiKeyStorage();
}

inline FString GetDatabasePath() {
  EnsureInitialized();
  return DatabasePathStorage();
}

inline int32 GetVectorDimension() {
  EnsureInitialized();
  return VectorDimensionStorage();
}

inline int32 GetMaxRecallResults() {
  EnsureInitialized();
  return MaxRecallResultsStorage();
}

inline FString GetSdkVersion() { return TEXT("0.6.3"); }

inline void SetApiConfig(const FString &ApiUrl, const FString &ApiKey) {
  EnsureInitialized();
  !ApiUrl.IsEmpty() ? (void)(ApiUrlStorage() = ApiUrl) : (void)0;
  ApiKeyStorage() = ApiKey;
}

inline void SetConfigFilePathOverride(const FString &ConfigFilePath) {
  ConfigFilePathOverrideStorage() = ConfigFilePath;
}

} // namespace SDKConfig
