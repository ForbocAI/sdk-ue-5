#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {

struct FMemoryRecord {
  FString Id;
  FString NpcId;
  FString Text;
  float Importance;

  FMemoryRecord() : Importance(0.5f) {}
};

struct FGameMemorySliceState {
  rtk::EntityState<FMemoryRecord> Entities;

  bool operator==(const FGameMemorySliceState &Other) const {
    return Entities.ids == Other.Entities.ids;
  }
};

} // namespace TestGame
