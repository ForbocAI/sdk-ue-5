#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {
namespace StealthActions {

/** User Story: As a features systems stealth consumer, I need to invoke set door open action creator through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline rtk::ActionCreator<bool> setDoorOpenActionCreator() */
inline rtk::ActionCreator<bool> setDoorOpenActionCreator() {
  static auto C =
      rtk::createAction<bool>(TEXT("testgame/stealth/setDoorOpen"));
  return C;
}

/** User Story: As a features systems stealth consumer, I need to invoke bump alert action creator through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline rtk::ActionCreator<int32> bumpAlertActionCreator() */
inline rtk::ActionCreator<int32> bumpAlertActionCreator() {
  static auto C =
      rtk::createAction<int32>(TEXT("testgame/stealth/bumpAlert"));
  return C;
}

/** User Story: As a features systems stealth consumer, I need to invoke set door open through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline rtk::AnyAction setDoorOpen(bool V) */
inline rtk::AnyAction setDoorOpen(bool V) {
  return setDoorOpenActionCreator()(V);
}

/** User Story: As a features systems stealth consumer, I need to invoke bump alert through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline rtk::AnyAction bumpAlert(int32 Delta) */
inline rtk::AnyAction bumpAlert(int32 Delta) {
  return bumpAlertActionCreator()(Delta);
}

} // namespace StealthActions
} // namespace TestGame
