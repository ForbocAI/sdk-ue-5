#pragma once

#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace MicroGame {
namespace ScenarioActions {

/** User Story: As a systems harness scenario consumer, I need required coverage and executable steps accepted as one event so contract generations cannot mix. @fn inline rtk::ActionCreator<FScenarioContractPayload> setContractActionCreator() */
inline rtk::ActionCreator<FScenarioContractPayload>
setContractActionCreator() {
  static const auto Creator = rtk::createAction<FScenarioContractPayload>(
      TEXT("microgame/scenario/contractReceived"));
  return Creator;
}

/** User Story: As a systems harness scenario consumer, I need required groups and scenario steps dispatched atomically so live API coverage cannot use a stale local list. @fn inline rtk::AnyAction setContract(FScenarioContractPayload Contract) */
inline rtk::AnyAction setContract(FScenarioContractPayload Contract) {
  return setContractActionCreator()(MoveTemp(Contract));
}

} // namespace ScenarioActions
} // namespace MicroGame
