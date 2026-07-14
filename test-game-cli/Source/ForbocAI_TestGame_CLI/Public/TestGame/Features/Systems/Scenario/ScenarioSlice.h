#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Scenario/ScenarioActions.h"
#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"

namespace TestGame {

inline rtk::Slice<FScenarioSliceState> CreateScenarioSlice() {
  FScenarioSliceState Initial;
  Initial.Steps = {};
  return rtk::createSlice<FScenarioSliceState>(
      TEXT("testgame/scenario"), Initial,
      [](rtk::ActionReducerMapBuilder<FScenarioSliceState> &Builder) {
        Builder.addCase(
            ScenarioActions::setContractActionCreator(),
            [](const FScenarioSliceState &,
               const rtk::Action<TArray<FScenarioStep>> &Action)
                -> FScenarioSliceState {
              FScenarioSliceState Next;
              Next.Steps = Action.PayloadValue;
              return Next;
            });
      });
}

} // namespace TestGame
