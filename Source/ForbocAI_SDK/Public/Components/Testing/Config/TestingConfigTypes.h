#pragma once

#include "CoreMinimal.h"

namespace TestingConfigAdapters {

struct FTestingConfigValues {
  FString Empty;
  FString ExplicitApiUrl;
  FString NormalizedExplicitApiUrl;
  FString ApiKey;
  FString MissingConfigPath;
  bool bLocalAvailable;
  bool bLocalUnavailable;
};

struct FTestingConfigCounts {
  int32 None;
  int32 Increment;
  int32 Once;
};

struct FTestingConfigAssertions {
  FString ExplicitUrl;
  FString ExplicitSource;
  FString ExplicitProbe;
  FString LocalUrl;
  FString LocalSource;
  FString LocalProbe;
  FString ProductionUrl;
  FString ProductionSource;
  FString TimeoutProbe;
  FString ProductionKey;
  FString LocalKey;
  FString RedactedUrl;
  FString RedactedKey;
};

struct FTestingConfigData {
  FString AutomationName;
  FTestingConfigValues Values;
  FTestingConfigCounts Counts;
  FTestingConfigAssertions Assertions;
};

} // namespace TestingConfigAdapters
