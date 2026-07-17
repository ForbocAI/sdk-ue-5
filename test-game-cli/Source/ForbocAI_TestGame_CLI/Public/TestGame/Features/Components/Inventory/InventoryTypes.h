#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FInventoryState {
  TMap<FString, TArray<FString>> ByOwner;
};

} // namespace TestGame
