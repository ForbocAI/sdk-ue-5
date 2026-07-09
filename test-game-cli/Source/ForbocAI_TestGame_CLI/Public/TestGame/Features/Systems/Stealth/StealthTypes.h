#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FStealthState {
  bool bDoorOpen;
  int32 AlertLevel;

  FStealthState() : bDoorOpen(false), AlertLevel(0) {}
};

} // namespace TestGame
