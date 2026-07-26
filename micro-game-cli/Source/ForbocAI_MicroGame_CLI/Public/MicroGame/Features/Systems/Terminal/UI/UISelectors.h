#pragma once

#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
namespace UISelectors {

/** User Story: As a systems terminal ui consumer, I need to invoke select ui mode through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline FString SelectUiMode(const FUIState &State) */
inline FString SelectUiMode(const FUIState &State) { return State.Mode; }

/** User Story: As a systems terminal ui consumer, I need to invoke select ui messages through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline TArray<FString> SelectUiMessages(const FUIState &State) */
inline TArray<FString> SelectUiMessages(const FUIState &State) {
  return State.Messages;
}

} // namespace UISelectors
} // namespace MicroGame
