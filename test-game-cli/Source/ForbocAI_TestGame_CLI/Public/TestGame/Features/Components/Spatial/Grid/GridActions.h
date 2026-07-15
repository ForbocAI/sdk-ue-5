#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Spatial/Grid/GridTypes.h"

namespace TestGame {

struct FSetGridSizePayload {
  int32 Width;
  int32 Height;
};

namespace GridActions {

inline rtk::ActionCreator<FSetGridSizePayload> setGridSizeActionCreator() {
  static auto C =
      rtk::createAction<FSetGridSizePayload>(TEXT("testgame/grid/setGridSize"));
  return C;
}

inline rtk::ActionCreator<TArray<FPosition>> setBlockedActionCreator() {
  static auto C = rtk::createAction<TArray<FPosition>>(
      TEXT("testgame/grid/setBlocked"));
  return C;
}

inline rtk::AnyAction setGridSize(const FSetGridSizePayload &P) {
  return setGridSizeActionCreator()(P);
}

inline rtk::AnyAction setBlocked(const TArray<FPosition> &B) {
  return setBlockedActionCreator()(B);
}

} // namespace GridActions
} // namespace TestGame
