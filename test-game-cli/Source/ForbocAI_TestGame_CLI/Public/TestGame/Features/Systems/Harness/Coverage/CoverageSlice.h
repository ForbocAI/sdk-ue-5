#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {

inline rtk::Slice<FHarnessState> CreateHarnessSlice() {
  return rtk::createSlice<FHarnessState>(
      TEXT("testgame/harness"), FHarnessState(),
      [](rtk::ActionReducerMapBuilder<FHarnessState> &Builder) {
        Builder.addCase(
            HarnessActions::markCoveredActionCreator(),
            [](const FHarnessState &S,
               const rtk::Action<ECommandGroup> &A) -> FHarnessState {
              FHarnessState Next = S;
              Next.Covered.Add(A.PayloadValue, true);
              return Next;
            });
        Builder.addCase(
            HarnessActions::resetCoverageActionCreator(),
            [](const FHarnessState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FHarnessState {
              return FHarnessState();
            });
      });
}

} // namespace TestGame
