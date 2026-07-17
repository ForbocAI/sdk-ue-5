#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {

struct FMemoryRecord {
  FString Id;
  FString NpcId;
  FString Text;
  float Importance{};
};

struct FGameMemorySliceState {
  rtk::EntityState<FMemoryRecord> Entities;

  /** User Story: As a features systems memory consumer, I need to compare values for equality through a stable signature so the features systems memory workflow remains explicit and composable. @fn bool operator==(const FGameMemorySliceState &Other) const */
  bool operator==(const FGameMemorySliceState &Other) const {
    return Entities.ids == Other.Entities.ids;
  }
};

} // namespace TestGame
