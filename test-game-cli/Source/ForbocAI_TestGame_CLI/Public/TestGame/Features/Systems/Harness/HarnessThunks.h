#pragma once
#include "CoreMinimal.h"

namespace TestGame {

inline FString ResolveRuntimeUrl() {
  return FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_URL"));
}

} // namespace TestGame
