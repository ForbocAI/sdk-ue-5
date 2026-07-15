#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Components/Spatial/SpatialTypes.h"

namespace TestGame {

struct FGridState {
  int32 Width{};
  int32 Height{};
  TArray<FPosition> Blocked;
};

} // namespace TestGame
