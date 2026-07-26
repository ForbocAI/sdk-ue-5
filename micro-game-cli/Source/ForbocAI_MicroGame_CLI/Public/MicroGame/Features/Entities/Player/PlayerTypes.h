#pragma once

#include "CoreMinimal.h"
#include "MicroGame/Features/Components/Spatial/SpatialTypes.h"

namespace MicroGame {

struct FPlayerState {
  FString Name;
  int32 Hp{};
  bool bHidden{};
  FPosition Position;
  TArray<FString> Inventory;
};

} // namespace MicroGame
