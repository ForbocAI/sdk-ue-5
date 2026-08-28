#pragma once

#include "Systems/CLI/CommandResult/CommandResultAdapters.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace CLIOps::NpcCommands {

using NpcResult = CLIOps::CommandResult::Result;

/** User Story: As NPC creation, I need the CLI to validate and present one SDK actor creation operation. @fn inline NpcResult CreateNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
inline NpcResult CreateNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Creating, Args[First]));
        const FNPCInternalState Npc = Ops::createNpc(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Created, Npc.Id));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.State, Npc.State.JsonData));
        return CLIOps::CommandResult::Success(Npc.Id);
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.CreateUsage);
      });
}

/** User Story: As NPC hydration, I need the CLI to present one SDK actor/vector recall operation. @fn inline NpcResult RecallNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
inline NpcResult RecallNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        return func::match(
            Ops::recallNpc(Store, Args[First]),
            [&](const FNPCInternalState &Actor) {
              ForbocAI::CLI::Presentation::logCliMessage(
                  formatCliMessage(State.Messages.Recalled, Actor.Id));
              ForbocAI::CLI::Presentation::logCliMessage(
                  formatCliMessage(State.Messages.Persona, Actor.Persona));
              return CLIOps::CommandResult::Success(Actor.Id);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  formatCliMessage(State.Messages.NotFound, Args[First]));
            });
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.RecallUsage);
      });
}

/** User Story: As NPC state inspection, I need explicit or active actor state selected by one SDK operation and rendered by the CLI. @fn inline NpcResult PrintNpcState(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
inline NpcResult PrintNpcState(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const bool bHasExplicitNpc =
      Args.Num() >= State.Limits.SingleArgumentCount;
  const func::Maybe<FNPCInternalState> Target =
      CLIOps::CommandRouting::matchCondition<func::Maybe<FNPCInternalState>>(
          bHasExplicitNpc, [&]() { return Ops::getNpc(Store, Args[First]); },
          [&]() { return Ops::getActiveNpc(Store); });
  return func::match(
      Target,
      [&State](const FNPCInternalState &Npc) {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Id, Npc.Id));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Persona, Npc.Persona));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.State, Npc.State.JsonData));
        return CLIOps::CommandResult::Success(State.Messages.StatePrinted);
      },
      [&]() {
        return CLIOps::CommandResult::Failure(
            bHasExplicitNpc
                ? formatCliMessage(State.Messages.NotFound, Args[First])
                : State.Messages.NoActive);
      });
}

/** User Story: As NPC state mutation, I need CLI options decoded once and delegated to the SDK for target resolution and reduction. @fn inline NpcResult UpdateNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State) */
inline NpcResult UpdateNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return func::match(
      ForbocAI::CLI::NPC::decodeNpcUpdate(Args, State),
      [&Store, &State](const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded) {
        const FActorUpdateResult Updated = Ops::updateNpc(
            Store, FActorUpdateInput{Decoded.RequestedNpcId, Decoded.Delta});
        return func::match(
            Updated.TargetActorId,
            [&](const FString &TargetId) {
              return func::match(
                  Updated.Actor,
                  [&](const FNPCInternalState &) {
                    ForbocAI::CLI::Presentation::logCliMessage(
                        formatCliMessage(State.Messages.Updated, TargetId,
                                         Decoded.Delta.JsonData));
                    return CLIOps::CommandResult::Success(
                        State.Messages.UpdateDone);
                  },
                  [&]() {
                    return CLIOps::CommandResult::Failure(
                        formatCliMessage(State.Messages.NotFound, TargetId));
                  });
            },
            [&]() {
              return CLIOps::CommandResult::Failure(State.Messages.MissingId);
            });
      },
      [&State]() {
        return CLIOps::CommandResult::Failure(State.Messages.UpdateUsage);
      });
}

} // namespace CLIOps::NpcCommands
