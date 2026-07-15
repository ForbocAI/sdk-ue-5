#pragma once

#include "TestGame/Features/Systems/Bridge/BridgeAdapters.h"

namespace TestGame {

struct FJumpValidation {
  bool bValid{};
  FString Reason;
};

struct FMoveDistanceResult {
  int32 AllowedDistance{};
  bool bCapped{};
};

inline FJumpValidation ValidateJump(const FBridgeState &Rules, int32 Force) {
  return Force > Rules.MaxJumpForce
             ? FJumpValidation{
                   false, FormatJumpForceExceeded(Force, Rules.MaxJumpForce)}
             : FJumpValidation{true, FString()};
}

inline FMoveDistanceResult CapMoveDistance(const FBridgeState &Rules,
                                           int32 RequestedDistance) {
  return {FMath::Min(RequestedDistance, Rules.MaxMoveDistance),
          RequestedDistance > Rules.MaxMoveDistance};
}

} // namespace TestGame
