#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Entities/Spatial/SpatialTypes.h"

namespace TestGame {

struct FPlayerState {
  FString Name;
  int32 Hp;
  bool bHidden;
  FPosition Position;
  TArray<FString> Inventory;

  FPlayerState() : Name(TEXT("Scout")), Hp(100), bHidden(true), Position(1, 1) {
    Inventory.Add(TEXT("coin-pouch"));
  }
};

} // namespace TestGame
