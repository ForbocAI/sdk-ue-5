#pragma once

#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioSelectors {

inline TArray<FScenarioStep>
SelectScenarioSteps(const FScenarioSliceState &State) {
  return State.Steps;
}

} // namespace ScenarioSelectors
} // namespace TestGame
