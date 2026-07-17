#pragma once

#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioSelectors {

/** User Story: As a systems harness scenario consumer, I need to invoke select scenario steps through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline TArray<FScenarioStep> SelectScenarioSteps(const FScenarioSliceState &State) */
inline TArray<FScenarioStep>
SelectScenarioSteps(const FScenarioSliceState &State) {
  return State.Steps;
}

/** User Story: As an SDK verifier, I need required groups selected from the active API contract so newly required CLI behavior cannot be omitted from coverage. @fn inline TArray<FString> SelectRequiredCommandGroups(const FScenarioSliceState &State) */
inline TArray<FString>
SelectRequiredCommandGroups(const FScenarioSliceState &State) {
  return State.RequiredCommandGroups;
}

} // namespace ScenarioSelectors
} // namespace TestGame
