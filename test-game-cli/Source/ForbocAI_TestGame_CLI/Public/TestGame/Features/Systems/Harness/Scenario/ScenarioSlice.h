#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {

/** User Story: As a systems harness scenario consumer, I need to invoke create scenario slice through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn inline rtk::Slice<FScenarioSliceState> CreateScenarioSlice() */
inline rtk::Slice<FScenarioSliceState> CreateScenarioSlice() {
  return rtk::createSlice<FScenarioSliceState>(
      TEXT("testgame/scenario"), FScenarioSliceState(),
      [](rtk::ActionReducerMapBuilder<FScenarioSliceState> &Builder) {
        Builder.addCase(
            ScenarioActions::setContractActionCreator(),
            [](const FScenarioSliceState &,
               const rtk::Action<FScenarioContractPayload> &Action)
                -> FScenarioSliceState {
              FScenarioSliceState Next;
              Next.RequiredCommandGroups =
                  Action.PayloadValue.RequiredCommandGroups;
              Next.Steps = Action.PayloadValue.Steps;
              return Next;
            });
      });
}

} // namespace TestGame
