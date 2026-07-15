#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace TestGame {

struct FHarnessState {
  TMap<ECommandGroup, bool> Covered;
};

} // namespace TestGame
