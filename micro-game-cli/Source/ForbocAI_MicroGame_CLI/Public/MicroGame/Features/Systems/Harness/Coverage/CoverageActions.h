#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace MicroGame {
namespace CoverageActions {

/** User Story: As a systems harness coverage consumer, I need to invoke mark covered action creator through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> markCoveredActionCreator() */
inline rtk::ActionCreator<FString> markCoveredActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("microgame/harness/markCovered"));
  return C;
}

/** User Story: As a systems harness coverage consumer, I need to invoke reset coverage action creator through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload resetCoverageActionCreator() */
inline rtk::ActionCreatorWithoutPayload resetCoverageActionCreator() {
  static auto C =
      rtk::createAction(TEXT("microgame/harness/resetCoverage"));
  return C;
}

/** User Story: As a systems harness coverage consumer, I need to invoke mark covered through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::AnyAction markCovered(FString Group) */
inline rtk::AnyAction markCovered(FString Group) {
  return markCoveredActionCreator()(MoveTemp(Group));
}

/** User Story: As a systems harness coverage consumer, I need to invoke reset coverage through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::AnyAction resetCoverage() */
inline rtk::AnyAction resetCoverage() {
  return resetCoverageActionCreator()();
}

} // namespace CoverageActions
} // namespace MicroGame
