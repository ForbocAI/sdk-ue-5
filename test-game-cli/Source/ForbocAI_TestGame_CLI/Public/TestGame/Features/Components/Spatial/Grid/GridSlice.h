#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Spatial/Grid/GridActions.h"
#include "TestGame/Features/Components/Spatial/Grid/GridAdapters.h"

namespace TestGame {

namespace GridSelectors {
inline int32 SelectGridWidth(const FGridState &S) { return S.Width; }
inline int32 SelectGridHeight(const FGridState &S) { return S.Height; }
inline TArray<FPosition> SelectGridBlocked(const FGridState &S) {
  return S.Blocked;
}
} // namespace GridSelectors

inline rtk::Slice<FGridState> CreateGridSlice() {
  return rtk::createSlice<FGridState>(
      TEXT("testgame/grid"), CreateGridInitialState(),
      [](rtk::ActionReducerMapBuilder<FGridState> &Builder) {
        Builder.addCase(
            GridActions::setGridSizeActionCreator(),
            [](const FGridState &S,
               const rtk::Action<FSetGridSizePayload> &A) -> FGridState {
              FGridState Next = S;
              Next.Width = A.PayloadValue.Width;
              Next.Height = A.PayloadValue.Height;
              return Next;
            });
        Builder.addCase(
            GridActions::setBlockedActionCreator(),
            [](const FGridState &S,
               const rtk::Action<TArray<FPosition>> &A) -> FGridState {
              FGridState Next = S;
              Next.Blocked = A.PayloadValue;
              return Next;
            });
      });
}

} // namespace TestGame
