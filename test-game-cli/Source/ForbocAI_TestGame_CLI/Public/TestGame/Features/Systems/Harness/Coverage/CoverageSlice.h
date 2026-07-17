#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {

/** User Story: As a systems harness coverage consumer, I need to invoke create harness slice through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::Slice<FHarnessState> CreateHarnessSlice() */
inline rtk::Slice<FHarnessState> CreateHarnessSlice() {
  return rtk::createSlice<FHarnessState>(
      TEXT("testgame/harness"), FHarnessState(),
      [](rtk::ActionReducerMapBuilder<FHarnessState> &Builder) {
        Builder.addCase(
            CoverageActions::markCoveredActionCreator(),
            [](const FHarnessState &S,
               const rtk::Action<FString> &A) -> FHarnessState {
              FHarnessState Next = S;
              Next.Covered.Add(A.PayloadValue, true);
              return Next;
            });
        Builder.addCase(
            CoverageActions::resetCoverageActionCreator(),
            [](const FHarnessState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FHarnessState {
              return FHarnessState();
            });
      });
}

} // namespace TestGame
