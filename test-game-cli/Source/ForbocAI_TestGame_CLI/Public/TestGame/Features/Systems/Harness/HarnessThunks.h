#pragma once
#include "CoreMinimal.h"

namespace TestGame {

/** User Story: As a features systems harness consumer, I need to invoke resolve runtime url through a stable signature so the features systems harness workflow remains explicit and composable. @fn inline FString ResolveRuntimeUrl() */
inline FString ResolveRuntimeUrl() {
  return FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_URL"));
}

} // namespace TestGame
