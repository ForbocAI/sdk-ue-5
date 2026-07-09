#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FBridgeRulesState {
  int32 MaxJumpForce;
  int32 MaxMoveDistance;
  FString ActivePreset;

  FBridgeRulesState()
      : MaxJumpForce(500), MaxMoveDistance(2), ActivePreset(TEXT("default")) {}
};

typedef FBridgeRulesState FBridgeState;

} // namespace TestGame
