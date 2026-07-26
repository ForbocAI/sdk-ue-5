#pragma once

#include "CoreMinimal.h"
#include "MicroGame/Features/Components/Spatial/SpatialTypes.h"

namespace MicroGame {

struct FGridState {
  int32 Width{};
  int32 Height{};
  TArray<FPosition> Blocked;
};

} // namespace MicroGame
