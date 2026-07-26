#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace ScenarioSelectors {

/** User Story: As a systems harness scenario consumer, I need to invoke select scenario steps through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline TArray<FScenarioStep> SelectScenarioSteps(const FScenarioSliceState &State) */
inline TArray<FScenarioStep>
SelectScenarioSteps(const FScenarioSliceState &State) {
  return State.Steps;
}

/** User Story: As a focused test-game mode, I need API-owned scenarios projected to one required CLI command group without duplicating command data. @fn inline TArray<FScenarioStep> SelectScenarioStepsByCommandGroup( const FScenarioSliceState &State, const FString &Group) */
inline TArray<FScenarioStep> SelectScenarioStepsByCommandGroup(
    const FScenarioSliceState &State, const FString &Group) {
  return func::filter_array<FScenarioStep>(
      func::map_array<FScenarioStep, FScenarioStep>(
          State.Steps, [&Group](const FScenarioStep &Step) {
            FScenarioStep Selected = Step;
            Selected.Commands = func::filter_array<FCommandSpec>(
                Step.Commands, [&Group](const FCommandSpec &Command) {
                  return Command.Group == Group;
                });
            return Selected;
          }),
      [](const FScenarioStep &Step) { return !Step.Commands.IsEmpty(); });
}

/** User Story: As an SDK verifier, I need required groups selected from the active API contract so newly required CLI behavior cannot be omitted from coverage. @fn inline TArray<FString> SelectRequiredCommandGroups(const FScenarioSliceState &State) */
inline TArray<FString>
SelectRequiredCommandGroups(const FScenarioSliceState &State) {
  return State.RequiredCommandGroups;
}

} // namespace ScenarioSelectors
} // namespace TestGame
