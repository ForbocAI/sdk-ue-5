#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FInventoryState {
  TMap<FString, TArray<FString>> ByOwner;

  FInventoryState() {
    ByOwner.Add(TEXT("player"), {TEXT("coin-pouch")});
    ByOwner.Add(TEXT("miller"), {TEXT("medkit")});
  }
};

} // namespace TestGame
