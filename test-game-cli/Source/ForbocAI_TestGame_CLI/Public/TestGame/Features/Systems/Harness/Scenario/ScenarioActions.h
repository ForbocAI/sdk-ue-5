#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioActions {

/** User Story: As a systems harness scenario consumer, I need to invoke set contract action creator through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline rtk::ActionCreator<TArray<FScenarioStep>> setContractActionCreator() */
inline rtk::ActionCreator<TArray<FScenarioStep>> setContractActionCreator() {
  static const auto Creator = rtk::createAction<TArray<FScenarioStep>>(
      TEXT("testgame/scenario/contractReceived"));
  return Creator;
}

/** User Story: As a systems harness scenario consumer, I need to invoke contract received through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline rtk::AnyAction contractReceived(TArray<FScenarioStep> Steps) */
inline rtk::AnyAction contractReceived(TArray<FScenarioStep> Steps) {
  return setContractActionCreator()(MoveTemp(Steps));
}

} // namespace ScenarioActions
} // namespace TestGame
