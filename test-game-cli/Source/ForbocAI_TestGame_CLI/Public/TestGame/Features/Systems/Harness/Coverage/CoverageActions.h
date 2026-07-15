#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace TestGame {
namespace HarnessActions {

inline rtk::ActionCreator<ECommandGroup> markCoveredActionCreator() {
  static auto C =
      rtk::createAction<ECommandGroup>(TEXT("testgame/harness/markCovered"));
  return C;
}

inline rtk::ActionCreatorWithoutPayload resetCoverageActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/harness/resetCoverage"));
  return C;
}

inline rtk::AnyAction markCovered(ECommandGroup G) {
  return markCoveredActionCreator()(G);
}

inline rtk::AnyAction resetCoverage() {
  return resetCoverageActionCreator()();
}

} // namespace HarnessActions
} // namespace TestGame
