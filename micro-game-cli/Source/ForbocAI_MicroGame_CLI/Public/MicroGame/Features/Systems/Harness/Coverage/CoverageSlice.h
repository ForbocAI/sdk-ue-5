#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace MicroGame {

/** User Story: As a systems harness coverage consumer, I need to invoke create harness slice through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline rtk::Slice<FHarnessState> CreateHarnessSlice() */
inline rtk::Slice<FHarnessState> CreateHarnessSlice() {
  return rtk::createSlice<FHarnessState>(
      TEXT("microgame/harness"), FHarnessState(),
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

} // namespace MicroGame
