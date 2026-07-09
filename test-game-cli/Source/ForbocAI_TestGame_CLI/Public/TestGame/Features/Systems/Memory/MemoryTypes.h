#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FMemoryRecord {
  FString Id;
  FString NpcId;
  FString Text;
  float Importance;

  FMemoryRecord() : Importance(0.5f) {}
};

} // namespace TestGame
