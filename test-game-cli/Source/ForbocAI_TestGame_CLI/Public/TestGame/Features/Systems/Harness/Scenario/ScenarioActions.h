#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioActions {

inline rtk::ActionCreator<TArray<FScenarioStep>> setContractActionCreator() {
  static const auto Creator = rtk::createAction<TArray<FScenarioStep>>(
      TEXT("testgame/scenario/contractReceived"));
  return Creator;
}

inline rtk::AnyAction contractReceived(TArray<FScenarioStep> Steps) {
  return setContractActionCreator()(MoveTemp(Steps));
}

} // namespace ScenarioActions
} // namespace TestGame
