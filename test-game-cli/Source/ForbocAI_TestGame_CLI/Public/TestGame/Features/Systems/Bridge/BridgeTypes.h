#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FBridgeRulesState {
  int32 MaxJumpForce{};
  int32 MaxMoveDistance{};
  FString ActivePreset;
};

typedef FBridgeRulesState FBridgeState;

} // namespace TestGame
