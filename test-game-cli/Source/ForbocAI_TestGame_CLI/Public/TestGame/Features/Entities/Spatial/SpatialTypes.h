#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FPosition {
  int32 X;
  int32 Y;

  FPosition() : X(0), Y(0) {}
  FPosition(int32 InX, int32 InY) : X(InX), Y(InY) {}

  bool operator==(const FPosition &Other) const {
    return X == Other.X && Y == Other.Y;
  }
};

} // namespace TestGame
