#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"

namespace SoulSlice::Actions {

/**
 * User Story: As a Soul workflow owner, I need one reducer-owned reset event so
 * package state can return to its canonical initial value.
 * @fn inline const rtk::ActionCreatorWithoutPayload &clearSoulStateActionCreator()
 */
inline const rtk::ActionCreatorWithoutPayload &clearSoulStateActionCreator() {
  static const rtk::ActionCreatorWithoutPayload ActionCreator =
      rtk::createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD0ACAD4923A6));
  return ActionCreator;
}

/**
 * User Story: As a Soul workflow caller, I need a typed reset action emitted
 * without mutating package state outside its reducer.
 * @fn inline rtk::AnyAction clearSoulState()
 */
inline rtk::AnyAction clearSoulState() {
  return clearSoulStateActionCreator()();
}

} // namespace SoulSlice::Actions
