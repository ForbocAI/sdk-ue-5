#pragma once

#include "CoreMinimal.h"

namespace CommandletInvocation {

struct FInvocation {
  FString Command;
  TArray<FString> Args;
  FString ApiUrl;
  FString ApiKey;
};

struct FInvocationTransportSettings {
  FString ArgumentParameterTemplate;
  FString IndexToken;
  int32 FirstArgumentIndex;
  int32 NextArgumentOffset;
};

} // namespace CommandletInvocation
