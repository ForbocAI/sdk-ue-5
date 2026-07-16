#pragma once

#include "CoreMinimal.h"

namespace AsyncAdapters {

struct FAsyncWaitTiming {
  double UnboundedTimeoutSeconds;
  float PollIntervalSeconds;
};

struct FAsyncWaitMessages {
  FString Timeout;
};

struct FAsyncWaitSettings {
  FAsyncWaitTiming Timing;
  FAsyncWaitMessages Messages;
};

} // namespace AsyncAdapters
