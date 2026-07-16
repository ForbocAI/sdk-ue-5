#pragma once

#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace TestGame {
namespace CommandRunnerSelectors {

/** User Story: As a systems harness command runner consumer, I need to invoke select npc aliases through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline const TMap<FString, FString> & SelectNpcAliases(const CommandRunner::FCommandAliasState &State) */
inline const TMap<FString, FString> &
SelectNpcAliases(const CommandRunner::FCommandAliasState &State) {
  return State.NpcAliases;
}

/** User Story: As a systems harness command runner consumer, I need to invoke select ghost session aliases through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline const TMap<FString, FString> & SelectGhostSessionAliases(const CommandRunner::FCommandAliasState &State) */
inline const TMap<FString, FString> &
SelectGhostSessionAliases(const CommandRunner::FCommandAliasState &State) {
  return State.GhostSessionAliases;
}

/** User Story: As a systems harness command runner consumer, I need to invoke select soul transaction aliases through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline const TMap<FString, FString> & SelectSoulTransactionAliases( const CommandRunner::FCommandAliasState &State) */
inline const TMap<FString, FString> &
SelectSoulTransactionAliases(
    const CommandRunner::FCommandAliasState &State) {
  return State.SoulTransactionAliases;
}

} // namespace CommandRunnerSelectors
} // namespace TestGame
