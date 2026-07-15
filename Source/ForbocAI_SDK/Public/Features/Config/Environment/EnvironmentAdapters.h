#pragma once

#include "Features/Config/State/StateAdapters.h"

#include "HAL/PlatformMisc.h"

namespace SDKConfig {

inline void LoadFromEnvironment() {
  const FString ApiUrl =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_URL"));
  const FString ApiKey =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_KEY"));
  const FString DatabasePath =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_DATABASE_PATH"));
  const FString VectorDimension =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_VECTOR_DIMENSION"));
  const FString MaxRecall =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_MAX_RECALL"));

  !ApiUrl.IsEmpty() ? (void)(ApiUrlStorage() = ApiUrl) : (void)0;
  !ApiKey.IsEmpty() ? (void)(ApiKeyStorage() = ApiKey) : (void)0;
  !DatabasePath.IsEmpty()
      ? (void)(DatabasePathStorage() = DatabasePath)
      : (void)0;
  !VectorDimension.IsEmpty()
      ? (void)(VectorDimensionStorage() = FCString::Atoi(*VectorDimension))
      : (void)0;
  !MaxRecall.IsEmpty()
      ? (void)(MaxRecallResultsStorage() = FCString::Atoi(*MaxRecall))
      : (void)0;
}

} // namespace SDKConfig
