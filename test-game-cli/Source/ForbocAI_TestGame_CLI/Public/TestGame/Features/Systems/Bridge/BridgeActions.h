#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"

namespace TestGame {
namespace GameBridgeActions {

struct FSetBridgeRulesPayload {
  int32 MaxJumpForce;
  bool bHasMaxJumpForce;
  int32 MaxMoveDistance;
  bool bHasMaxMoveDistance;
  FString ActivePreset;
  bool bHasActivePreset;

  FSetBridgeRulesPayload()
      : MaxJumpForce(0), bHasMaxJumpForce(false), MaxMoveDistance(0),
        bHasMaxMoveDistance(false), bHasActivePreset(false) {}
};

inline rtk::ActionCreator<FSetBridgeRulesPayload> setBridgeRulesActionCreator() {
  static auto C = rtk::createAction<FSetBridgeRulesPayload>(
      TEXT("testgame/bridge/setBridgeRules"));
  return C;
}

inline rtk::ActionCreator<FString> loadBridgePresetActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/bridge/loadBridgePreset"));
  return C;
}

inline rtk::AnyAction setBridgeRules(const FSetBridgeRulesPayload &P) {
  return setBridgeRulesActionCreator()(P);
}

inline rtk::AnyAction setBridgeRules(const FBridgeRulesState &R) {
  FSetBridgeRulesPayload Payload;
  Payload.MaxJumpForce = R.MaxJumpForce;
  Payload.bHasMaxJumpForce = true;
  Payload.MaxMoveDistance = R.MaxMoveDistance;
  Payload.bHasMaxMoveDistance = true;
  Payload.ActivePreset = R.ActivePreset;
  Payload.bHasActivePreset = true;
  return setBridgeRules(Payload);
}

inline rtk::AnyAction loadBridgePreset(const FString &P) {
  return loadBridgePresetActionCreator()(P);
}

} // namespace GameBridgeActions
} // namespace TestGame
