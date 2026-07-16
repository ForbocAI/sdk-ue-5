#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FPosition {
  int32 X;
  int32 Y;

  /** User Story: As a features components spatial consumer, I need to invoke fposition through a stable signature so the features components spatial workflow remains explicit and composable. @fn FPosition() = default */
  FPosition() = default;
  /** User Story: As a features components spatial consumer, I need to invoke fposition through a stable signature so the features components spatial workflow remains explicit and composable. @fn FPosition(int32 InX, int32 InY) */
  FPosition(int32 InX, int32 InY) : X(InX), Y(InY) {}

  /** User Story: As a features components spatial consumer, I need to compare values for equality through a stable signature so the features components spatial workflow remains explicit and composable. @fn bool operator==(const FPosition &Other) const */
  bool operator==(const FPosition &Other) const {
    return X == Other.X && Y == Other.Y;
  }
};

} // namespace TestGame
