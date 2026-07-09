#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace UIActions {

inline rtk::ActionCreator<EPlayMode> setModeActionCreator() {
  static auto C = rtk::createAction<EPlayMode>(TEXT("testgame/ui/setMode"));
  return C;
}

inline rtk::ActionCreator<FString> addMessageActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/ui/addMessage"));
  return C;
}

inline rtk::AnyAction setMode(EPlayMode M) {
  return setModeActionCreator()(M);
}

inline rtk::AnyAction addMessage(const FString &Msg) {
  return addMessageActionCreator()(Msg);
}

} // namespace UIActions
} // namespace TestGame
