#pragma once

#include "CoreMinimal.h"

namespace MicroGame {

struct FInventoryState {
  TMap<FString, TArray<FString>> ByOwner;
};

} // namespace MicroGame
