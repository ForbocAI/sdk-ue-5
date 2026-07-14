#pragma once

#include "CoreMinimal.h"

namespace CommandletInvocation {

struct FInvocation {
  FString Command;
  TArray<FString> Args;
  FString ApiUrl;
  FString ApiKey;
};

} // namespace CommandletInvocation
