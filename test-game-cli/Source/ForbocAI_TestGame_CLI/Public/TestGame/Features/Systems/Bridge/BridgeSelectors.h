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

/** User Story: As a features systems bridge consumer, I need to invoke validate jump through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline FJumpValidation ValidateJump(const FBridgeState &Rules, int32 Force) */
inline FJumpValidation ValidateJump(const FBridgeState &Rules, int32 Force) {
  return Force > Rules.MaxJumpForce
             ? FJumpValidation{
                   false, FormatJumpForceExceeded(Force, Rules.MaxJumpForce)}
             : FJumpValidation{true, FString()};
}

/** User Story: As a features systems bridge consumer, I need to invoke cap move distance through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline FMoveDistanceResult CapMoveDistance(const FBridgeState &Rules, int32 RequestedDistance) */
inline FMoveDistanceResult CapMoveDistance(const FBridgeState &Rules,
                                           int32 RequestedDistance) {
  return {FMath::Min(RequestedDistance, Rules.MaxMoveDistance),
          RequestedDistance > Rules.MaxMoveDistance};
}

} // namespace TestGame
