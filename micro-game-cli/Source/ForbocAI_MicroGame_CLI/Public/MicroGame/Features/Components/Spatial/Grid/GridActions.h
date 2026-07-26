#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Components/Spatial/Grid/GridTypes.h"

namespace MicroGame {

struct FSetGridSizePayload {
  int32 Width;
  int32 Height;
};

namespace GridActions {

/** User Story: As a components spatial grid consumer, I need to invoke set grid size action creator through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline rtk::ActionCreator<FSetGridSizePayload> setGridSizeActionCreator() */
inline rtk::ActionCreator<FSetGridSizePayload> setGridSizeActionCreator() {
  static auto C =
      rtk::createAction<FSetGridSizePayload>(TEXT("microgame/grid/setGridSize"));
  return C;
}

/** User Story: As a components spatial grid consumer, I need to invoke set blocked action creator through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline rtk::ActionCreator<TArray<FPosition>> setBlockedActionCreator() */
inline rtk::ActionCreator<TArray<FPosition>> setBlockedActionCreator() {
  static auto C = rtk::createAction<TArray<FPosition>>(
      TEXT("microgame/grid/setBlocked"));
  return C;
}

/** User Story: As a components spatial grid consumer, I need to invoke set grid size through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline rtk::AnyAction setGridSize(const FSetGridSizePayload &P) */
inline rtk::AnyAction setGridSize(const FSetGridSizePayload &P) {
  return setGridSizeActionCreator()(P);
}

/** User Story: As a components spatial grid consumer, I need to invoke set blocked through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline rtk::AnyAction setBlocked(const TArray<FPosition> &B) */
inline rtk::AnyAction setBlocked(const TArray<FPosition> &B) {
  return setBlockedActionCreator()(B);
}

} // namespace GridActions
} // namespace MicroGame
