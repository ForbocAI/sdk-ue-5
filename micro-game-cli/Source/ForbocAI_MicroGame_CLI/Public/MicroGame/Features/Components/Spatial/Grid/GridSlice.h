#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Components/Spatial/Grid/GridActions.h"
#include "MicroGame/Features/Components/Spatial/Grid/GridAdapters.h"

namespace MicroGame {

namespace GridSelectors {
/** User Story: As a components spatial grid consumer, I need to invoke select grid width through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline int32 SelectGridWidth(const FGridState &S) */
inline int32 SelectGridWidth(const FGridState &S) { return S.Width; }
/** User Story: As a components spatial grid consumer, I need to invoke select grid height through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline int32 SelectGridHeight(const FGridState &S) */
inline int32 SelectGridHeight(const FGridState &S) { return S.Height; }
/** User Story: As a components spatial grid consumer, I need to invoke select grid blocked through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline TArray<FPosition> SelectGridBlocked(const FGridState &S) */
inline TArray<FPosition> SelectGridBlocked(const FGridState &S) {
  return S.Blocked;
}
} // namespace GridSelectors

/** User Story: As a components spatial grid consumer, I need to invoke create grid slice through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline rtk::Slice<FGridState> CreateGridSlice() */
inline rtk::Slice<FGridState> CreateGridSlice() {
  return rtk::createSlice<FGridState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV7C71E8A84336), CreateGridInitialState(),
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

} // namespace MicroGame
