#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Bridge/BridgeTypes.h"

namespace MicroGame {
namespace GameBridgeActions {

struct FSetBridgeRulesPayload {
  int32 MaxJumpForce{};
  bool bHasMaxJumpForce{};
  int32 MaxMoveDistance{};
  bool bHasMaxMoveDistance{};
  FString ActivePreset;
  bool bHasActivePreset{};
};

/** User Story: As a features systems bridge consumer, I need to invoke set bridge rules action creator through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline rtk::ActionCreator<FSetBridgeRulesPayload> setBridgeRulesActionCreator() */
inline rtk::ActionCreator<FSetBridgeRulesPayload> setBridgeRulesActionCreator() {
  static auto C = rtk::createAction<FSetBridgeRulesPayload>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVB7860EFB315B));
  return C;
}

/** User Story: As a features systems bridge consumer, I need to invoke load bridge preset action creator through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> loadBridgePresetActionCreator() */
inline rtk::ActionCreator<FString> loadBridgePresetActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVFA7BC3E6207C));
  return C;
}

/** User Story: As a features systems bridge consumer, I need to invoke set bridge rules through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline rtk::AnyAction setBridgeRules(const FSetBridgeRulesPayload &P) */
inline rtk::AnyAction setBridgeRules(const FSetBridgeRulesPayload &P) {
  return setBridgeRulesActionCreator()(P);
}

/** User Story: As a features systems bridge consumer, I need to invoke set bridge rules through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline rtk::AnyAction setBridgeRules(const FBridgeRulesState &R) */
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

/** User Story: As a features systems bridge consumer, I need to invoke load bridge preset through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline rtk::AnyAction loadBridgePreset(const FString &P) */
inline rtk::AnyAction loadBridgePreset(const FString &P) {
  return loadBridgePresetActionCreator()(P);
}

} // namespace GameBridgeActions
} // namespace MicroGame
