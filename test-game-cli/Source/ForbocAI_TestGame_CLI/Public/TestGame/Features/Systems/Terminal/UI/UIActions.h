#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace UIActions {

/** User Story: As a systems terminal ui consumer, I need to invoke set mode action creator through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> setModeActionCreator() */
inline rtk::ActionCreator<FString> setModeActionCreator() {
  static auto C = rtk::createAction<FString>(TEXT("testgame/ui/setMode"));
  return C;
}

/** User Story: As a systems terminal ui consumer, I need to invoke add message action creator through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> addMessageActionCreator() */
inline rtk::ActionCreator<FString> addMessageActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/ui/addMessage"));
  return C;
}

/** User Story: As a systems terminal ui consumer, I need to invoke set mode through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::AnyAction setMode(FString Mode) */
inline rtk::AnyAction setMode(FString Mode) {
  return setModeActionCreator()(MoveTemp(Mode));
}

/** User Story: As a systems terminal ui consumer, I need to invoke add message through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::AnyAction addMessage(const FString &Msg) */
inline rtk::AnyAction addMessage(const FString &Msg) {
  return addMessageActionCreator()(Msg);
}

} // namespace UIActions
} // namespace TestGame
