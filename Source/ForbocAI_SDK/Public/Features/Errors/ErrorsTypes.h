#pragma once

#include "CoreMinimal.h"

namespace Errors {

enum class EErrorCategory : uint8 {
  Network,
  Http,
  Validation,
  Protocol,
  Memory,
  Inference,
  Arweave,
  Config,
  Unknown
};

struct FSDKError {
  EErrorCategory Category;
  FString Code;
  FString Message;
  int32 StatusCode;

  FSDKError()
      : Category(EErrorCategory::Unknown), Code(TEXT("UNKNOWN")),
        Message(TEXT("")), StatusCode(0) {}
};

} // namespace Errors
