#pragma once

#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioSelectors {

/** User Story: As a systems harness scenario consumer, I need to invoke select scenario steps through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline TArray<FScenarioStep> SelectScenarioSteps(const FScenarioSliceState &State) */
inline TArray<FScenarioStep>
SelectScenarioSteps(const FScenarioSliceState &State) {
  return State.Steps;
}

} // namespace ScenarioSelectors
} // namespace TestGame
