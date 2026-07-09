#pragma once
/**
 * Presentation-only terminal view helpers.
 */

#include "CoreMinimal.h"
#include "TestGame/Features/TestGameSelectors.h"

namespace TestGame {

inline FString RenderGrid(const FTerminalRenderState &State) {
  return State.GridText;
}

inline FString RenderLegend() {
  return TEXT("Legend :: P=Scout  D=Doomguard  M=Miller  #=Blocked  .=Open");
}

} // namespace TestGame
