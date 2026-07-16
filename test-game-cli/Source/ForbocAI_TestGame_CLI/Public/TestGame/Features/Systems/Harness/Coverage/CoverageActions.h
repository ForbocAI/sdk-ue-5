#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace TestGame {
namespace HarnessActions {

/** User Story: As a systems harness coverage consumer, I need to invoke mark covered action creator through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::ActionCreator<ECommandGroup> markCoveredActionCreator() */
inline rtk::ActionCreator<ECommandGroup> markCoveredActionCreator() {
  static auto C =
      rtk::createAction<ECommandGroup>(TEXT("testgame/harness/markCovered"));
  return C;
}

/** User Story: As a systems harness coverage consumer, I need to invoke reset coverage action creator through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload resetCoverageActionCreator() */
inline rtk::ActionCreatorWithoutPayload resetCoverageActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/harness/resetCoverage"));
  return C;
}

/** User Story: As a systems harness coverage consumer, I need to invoke mark covered through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::AnyAction markCovered(ECommandGroup G) */
inline rtk::AnyAction markCovered(ECommandGroup G) {
  return markCoveredActionCreator()(G);
}

/** User Story: As a systems harness coverage consumer, I need to invoke reset coverage through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::AnyAction resetCoverage() */
inline rtk::AnyAction resetCoverage() {
  return resetCoverageActionCreator()();
}

} // namespace HarnessActions
} // namespace TestGame
