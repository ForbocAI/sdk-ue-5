#pragma once

#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioSelectors {

inline TArray<FScenarioStep>
SelectScenarioSteps(const FScenarioSliceState &State) {
  return State.Steps;
}

} // namespace ScenarioSelectors
} // namespace TestGame
