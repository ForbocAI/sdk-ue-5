#pragma once

#include "CoreMinimal.h"

namespace TestingAsyncAdapters {

struct FTestingAsyncValues {
  int32 Immediate;
  int32 Late;
  double TimeoutSeconds;
};

struct FTestingAsyncAssertions {
  FString Immediate;
  FString Timeout;
  FString Callback;
  FString Late;
};

struct FTestingAsyncData {
  FString AutomationName;
  FTestingAsyncValues Values;
  FTestingAsyncAssertions Assertions;
};

} // namespace TestingAsyncAdapters
