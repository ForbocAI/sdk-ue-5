#pragma once

#include "CoreMinimal.h"

namespace SDKConfig {

inline constexpr TCHAR DEFAULT_API_URL[] = TEXT("https://api.forboc.ai");
inline constexpr int32 DEFAULT_VECTOR_DIMENSION = 384;
inline constexpr int32 DEFAULT_MAX_RECALL_RESULTS = 10;

/** User Story: As a features config state consumer, I need to invoke api url storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString &ApiUrlStorage() */
inline FString &ApiUrlStorage() {
  static FString Value = DEFAULT_API_URL;
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke api key storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString &ApiKeyStorage() */
inline FString &ApiKeyStorage() {
  static FString Value = TEXT("");
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke database path storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString &DatabasePathStorage() */
inline FString &DatabasePathStorage() {
  static FString Value = TEXT("");
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke vector dimension storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline int32 &VectorDimensionStorage() */
inline int32 &VectorDimensionStorage() {
  static int32 Value = DEFAULT_VECTOR_DIMENSION;
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke max recall results storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline int32 &MaxRecallResultsStorage() */
inline int32 &MaxRecallResultsStorage() {
  static int32 Value = DEFAULT_MAX_RECALL_RESULTS;
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke initialized storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline bool &InitializedStorage() */
inline bool &InitializedStorage() {
  static bool bInitialized = false;
  return bInitialized;
}

/** User Story: As a features config state consumer, I need to invoke config file path override storage through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString &ConfigFilePathOverrideStorage() */
inline FString &ConfigFilePathOverrideStorage() {
  static FString Value;
  return Value;
}

/** User Story: As a features config state consumer, I need to invoke reset to defaults through a stable signature so the features config state workflow remains explicit and composable. @fn inline void ResetToDefaults() */
inline void ResetToDefaults() {
  ApiUrlStorage() = DEFAULT_API_URL;
  ApiKeyStorage() = TEXT("");
  DatabasePathStorage() = TEXT("");
  VectorDimensionStorage() = DEFAULT_VECTOR_DIMENSION;
  MaxRecallResultsStorage() = DEFAULT_MAX_RECALL_RESULTS;
}

/** User Story: As a features config state consumer, I need to invoke initialize config through a stable signature so the features config state workflow remains explicit and composable. @fn inline void InitializeConfig() */
inline void InitializeConfig();
/** User Story: As a features config state consumer, I need to invoke ensure initialized through a stable signature so the features config state workflow remains explicit and composable. @fn inline void EnsureInitialized() */
inline void EnsureInitialized();
/** User Story: As a features config state consumer, I need to invoke reload config through a stable signature so the features config state workflow remains explicit and composable. @fn inline void ReloadConfig() */
inline void ReloadConfig();

/** User Story: As a features config state consumer, I need to invoke get api url through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString GetApiUrl() */
inline FString GetApiUrl() {
  EnsureInitialized();
  return ApiUrlStorage();
}

/** User Story: As a features config state consumer, I need to invoke get api key through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString GetApiKey() */
inline FString GetApiKey() {
  EnsureInitialized();
  return ApiKeyStorage();
}

/** User Story: As a features config state consumer, I need to invoke get database path through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString GetDatabasePath() */
inline FString GetDatabasePath() {
  EnsureInitialized();
  return DatabasePathStorage();
}

/** User Story: As a features config state consumer, I need to invoke get vector dimension through a stable signature so the features config state workflow remains explicit and composable. @fn inline int32 GetVectorDimension() */
inline int32 GetVectorDimension() {
  EnsureInitialized();
  return VectorDimensionStorage();
}

/** User Story: As a features config state consumer, I need to invoke get max recall results through a stable signature so the features config state workflow remains explicit and composable. @fn inline int32 GetMaxRecallResults() */
inline int32 GetMaxRecallResults() {
  EnsureInitialized();
  return MaxRecallResultsStorage();
}

/** User Story: As a features config state consumer, I need to invoke get sdk version through a stable signature so the features config state workflow remains explicit and composable. @fn inline FString GetSdkVersion() */
inline FString GetSdkVersion() { return TEXT("0.6.3"); }

/** User Story: As a features config state consumer, I need to invoke set api config through a stable signature so the features config state workflow remains explicit and composable. @fn inline void SetApiConfig(const FString &ApiUrl, const FString &ApiKey) */
inline void SetApiConfig(const FString &ApiUrl, const FString &ApiKey) {
  EnsureInitialized();
  !ApiUrl.IsEmpty() ? (void)(ApiUrlStorage() = ApiUrl) : (void)0;
  ApiKeyStorage() = ApiKey;
}

/** User Story: As a features config state consumer, I need to invoke set config file path override through a stable signature so the features config state workflow remains explicit and composable. @fn inline void SetConfigFilePathOverride(const FString &ConfigFilePath) */
inline void SetConfigFilePathOverride(const FString &ConfigFilePath) {
  ConfigFilePathOverrideStorage() = ConfigFilePath;
}

} // namespace SDKConfig
