#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {
namespace StealthActions {

inline rtk::ActionCreator<bool> setDoorOpenActionCreator() {
  static auto C =
      rtk::createAction<bool>(TEXT("testgame/stealth/setDoorOpen"));
  return C;
}

inline rtk::ActionCreator<int32> bumpAlertActionCreator() {
  static auto C =
      rtk::createAction<int32>(TEXT("testgame/stealth/bumpAlert"));
  return C;
}

inline rtk::AnyAction setDoorOpen(bool V) {
  return setDoorOpenActionCreator()(V);
}

inline rtk::AnyAction bumpAlert(int32 Delta) {
  return bumpAlertActionCreator()(Delta);
}

} // namespace StealthActions
} // namespace TestGame
