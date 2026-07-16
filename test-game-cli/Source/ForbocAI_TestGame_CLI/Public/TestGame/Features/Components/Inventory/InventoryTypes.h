#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FInventoryState {
  TMap<FString, TArray<FString>> ByOwner;

  /** User Story: As a features components inventory consumer, I need to invoke finventory state through a stable signature so the features components inventory workflow remains explicit and composable. @fn FInventoryState() */
  FInventoryState() {
    ByOwner.Add(TEXT("player"), {TEXT("coin-pouch")});
    ByOwner.Add(TEXT("miller"), {TEXT("medkit")});
  }
};

} // namespace TestGame
