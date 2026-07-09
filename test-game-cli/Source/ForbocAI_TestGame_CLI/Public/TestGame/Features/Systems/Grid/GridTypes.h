#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Entities/Spatial/SpatialTypes.h"

namespace TestGame {

struct FGridState {
  int32 Width;
  int32 Height;
  TArray<FPosition> Blocked;

  FGridState() : Width(8), Height(8) {
    Blocked.Add(FPosition(4, 4));
    Blocked.Add(FPosition(4, 5));
    Blocked.Add(FPosition(6, 2));
  }
};

} // namespace TestGame
