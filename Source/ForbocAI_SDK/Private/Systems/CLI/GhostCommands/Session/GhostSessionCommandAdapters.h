#pragma once

#include "Systems/CLI/CommandResult/CommandResultAdapters.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"

namespace CLIOps::GhostCommands {

using GhostResult = CLIOps::CommandResult::Result;

/** User Story: As every session-bound Ghost command, I need the required session identifier checked before exactly one Ghost SDK operation is composed. @fn template <typename Operation> inline GhostResult WithRequiredSession(const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First, Operation Run) */
template <typename Operation>
inline GhostResult WithRequiredSession(
    const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &State, int32 First,
    Operation Run) {
  return CLIOps::CommandRouting::matchCondition<GhostResult>(
      Args.Num() > First, [&]() { return Run(Args[First]); },
      [&]() {
        return CLIOps::CommandResult::Failure(State.RequiredSession);
      });
}

} // namespace CLIOps::GhostCommands
