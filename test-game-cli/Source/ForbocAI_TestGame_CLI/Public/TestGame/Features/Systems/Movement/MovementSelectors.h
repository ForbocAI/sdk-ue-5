#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"
#include "TestGame/Features/Systems/Grid/GridTypes.h"

namespace TestGame {

struct FJumpResult {
  bool bValid;
  FString Reason;
};

struct FMoveResult {
  int32 AllowedDistance;
  bool bCapped;
};

inline bool IsPassable(const FGridState &Grid, const FPosition &Pos) {
  struct CheckBlocked {
    static bool apply(const TArray<FPosition> &Blocked, const FPosition &P,
                      int32 Idx) {
      return Idx >= Blocked.Num() ? true
           : Blocked[Idx] == P   ? false
                                  : apply(Blocked, P, Idx + 1);
    }
  };
  return (Pos.X < 0 || Pos.Y < 0 || Pos.X >= Grid.Width ||
          Pos.Y >= Grid.Height)
             ? false
             : CheckBlocked::apply(Grid.Blocked, Pos, 0);
}

inline FJumpResult ValidateJump(const FBridgeRulesState &Rules, int32 Force) {
  FJumpResult R;
  R.bValid = Force <= Rules.MaxJumpForce;
  R.Reason = !R.bValid
                 ? FString::Printf(TEXT("Force %d exceeds max %d"), Force,
                                    Rules.MaxJumpForce)
                 : FString();
  return R;
}

inline FMoveResult CapMoveDistance(const FBridgeRulesState &Rules,
                                   int32 Requested) {
  FMoveResult R;
  R.AllowedDistance =
      Requested > Rules.MaxMoveDistance ? Rules.MaxMoveDistance : Requested;
  R.bCapped = R.AllowedDistance < Requested;
  return R;
}

} // namespace TestGame
