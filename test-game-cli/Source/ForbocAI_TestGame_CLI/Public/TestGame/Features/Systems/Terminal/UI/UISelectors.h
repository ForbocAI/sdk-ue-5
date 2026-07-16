#pragma once

#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace UISelectors {

/** User Story: As a systems terminal ui consumer, I need to invoke select ui mode through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline EPlayMode SelectUiMode(const FUIState &State) */
inline EPlayMode SelectUiMode(const FUIState &State) { return State.Mode; }

/** User Story: As a systems terminal ui consumer, I need to invoke select ui messages through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline TArray<FString> SelectUiMessages(const FUIState &State) */
inline TArray<FString> SelectUiMessages(const FUIState &State) {
  return State.Messages;
}

} // namespace UISelectors
} // namespace TestGame
