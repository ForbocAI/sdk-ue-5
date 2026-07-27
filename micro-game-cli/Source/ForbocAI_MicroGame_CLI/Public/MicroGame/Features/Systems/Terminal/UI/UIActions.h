#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
namespace UIActions {

/** User Story: As a systems terminal ui consumer, I need to invoke set mode action creator through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> setModeActionCreator() */
inline rtk::ActionCreator<FString> setModeActionCreator() {
  static auto C = rtk::createAction<FString>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVC4723F3DE242));
  return C;
}

/** User Story: As a systems terminal ui consumer, I need to invoke add message action creator through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> addMessageActionCreator() */
inline rtk::ActionCreator<FString> addMessageActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVFEB06D6E4F94));
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
} // namespace MicroGame
