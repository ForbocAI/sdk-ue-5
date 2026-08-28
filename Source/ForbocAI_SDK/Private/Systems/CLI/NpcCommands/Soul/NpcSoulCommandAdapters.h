#pragma once

#include "Systems/CLI/CommandResult/CommandResultAdapters.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace CLIOps::NpcCommands {

using NpcResult = CLIOps::CommandResult::Result;

/** User Story: As NPC soul import, I need one transaction identifier delegated to the SDK soul operation and its result rendered. @fn inline NpcResult ImportNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
inline NpcResult ImportNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        const FImportedNpc Npc = Ops::importNpcFromSoul(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Imported, Npc.NpcId));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Persona, Npc.Persona));
        return CLIOps::CommandResult::Success(State.Messages.ImportDone);
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.ImportUsage);
      });
}

} // namespace CLIOps::NpcCommands
