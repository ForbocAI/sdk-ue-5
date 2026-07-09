#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"

namespace TestGame {

struct FScenarioSliceState {
  TArray<FScenarioStep> Steps;

  bool operator==(const FScenarioSliceState &O) const {
    return Steps.Num() == O.Steps.Num();
  }
};

typedef FScenarioSliceState FScenarioState;

inline rtk::Slice<FScenarioSliceState> CreateScenarioSlice() {
  FScenarioSliceState Initial;
  Initial.Steps = {};
  return rtk::createSlice<FScenarioSliceState>(
      TEXT("testgame/scenario"), Initial,
      [](rtk::ActionReducerMapBuilder<FScenarioSliceState> &) {});
}

namespace ScenarioSelectors {
inline TArray<FScenarioStep> SelectScenarioSteps(const FScenarioSliceState &S) {
  return S.Steps;
}
} // namespace ScenarioSelectors

} // namespace TestGame
