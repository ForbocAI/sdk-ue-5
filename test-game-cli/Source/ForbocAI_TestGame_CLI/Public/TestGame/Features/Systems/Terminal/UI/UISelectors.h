#pragma once

#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace UISelectors {

inline EPlayMode SelectUiMode(const FUIState &State) { return State.Mode; }

inline TArray<FString> SelectUiMessages(const FUIState &State) {
  return State.Messages;
}

} // namespace UISelectors
} // namespace TestGame
