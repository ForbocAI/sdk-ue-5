#pragma once
#include "CoreMinimal.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"

namespace MicroGame {

/** User Story: As a features systems harness consumer, I need to invoke resolve runtime url through a stable signature so the features systems harness workflow remains explicit and composable. @fn inline FString ResolveRuntimeUrl() */
inline FString ResolveRuntimeUrl() {
  return FPlatformMisc::GetEnvironmentVariable(
      *VerificationAdapters::GameData().environment.runtimeUrlKey);
}

} // namespace MicroGame
