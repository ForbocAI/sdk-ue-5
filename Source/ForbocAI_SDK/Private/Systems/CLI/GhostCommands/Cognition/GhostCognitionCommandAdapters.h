#pragma once

#include "Systems/CLI/Analysis/CLIAnalysisAdapters.h"
#include "Systems/CLI/Ghost/CLIGhostThunks.h"
#include "Systems/CLI/GhostCommands/Session/GhostSessionCommandAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/CLI/NPC/Decision/NPCDecisionAdapters.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace CLIOps::GhostCommands {

/** User Story: As Ghost vector evidence storage, I need actor, observation, and optional type parsed before one Ghost SDK memory operation. @fn inline GhostResult StoreGhostMemory(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState, int32 First, int32 Second) */
inline GhostResult StoreGhostMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState,
    const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState, int32 First,
    int32 Second) {
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        const ForbocAI::CLI::NPC::FCLIOptionExtraction Type =
            ForbocAI::CLI::NPC::extractOptionValue(
                func::slice_array<FString>(Args, Second,
                                           Args.Num() - Second),
                ForbocAI::CLI::NPC::optionSpec(
                    MemoryState.Messages.TypeOption, ActorState));
        return CLIOps::CommandRouting::matchCondition<GhostResult>(
            Type.Rest.Num() >= MemoryState.Limits.DoubleArgumentCount,
            [&]() {
              const int32 ObservationIndex =
                  ActorState.Limits.FirstArgumentIndex +
                  ActorState.Limits.ValueOffset;
              const FString Observation =
                  ForbocAI::CLI::NPC::joinNpcArguments(
                      Type.Rest, ObservationIndex,
                      ActorState.Syntax.MessageSeparator);
              const FString MemoryType =
                  Type.Value.hasValue ? Type.Value.value : FString();
              Ops::storeGhostMemory(
                  Store,
                  FGhostMemoryStoreInput{
                      SessionId,
                      Type.Rest[ActorState.Limits.FirstArgumentIndex],
                      Observation, MemoryType});
              ForbocAI::CLI::Presentation::logCliMessage(
                  MemoryState.Messages.Stored);
              return CLIOps::CommandResult::Success(
                  MemoryState.Messages.StoreDone);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  MemoryState.Messages.StoreUsage);
            });
      });
}

/** User Story: As Ghost decision making, I need ordinary decision arguments delegated only to the Ghost SDK process operation and API-owned analytical results rendered verbatim. @fn inline GhostResult DecideGhost(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second) */
inline GhostResult DecideGhost(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        const ForbocAI::CLI::NPC::FCLINPCDecision Decision =
            ForbocAI::CLI::NPC::decodeNpcDecisionCommand(
                func::slice_array<FString>(Args, Second,
                                           Args.Num() - Second),
                ActorState);
        return CLIOps::CommandRouting::matchCondition<GhostResult>(
            !Decision.NpcId.IsEmpty() && !Decision.Observation.IsEmpty(),
            [&]() {
              FGhostProcessInput Input;
              Input.SessionId = SessionId;
              Input.Process =
                  ForbocAI::CLI::NPC::toProtocolProcessInput(Decision);
              const FAgentResponse Response = Ops::decideGhost(Store, Input);
              const FString Dialogue =
                  ForbocAI::CLI::NPC::selectDialogue(Response, ActorState);
              ForbocAI::CLI::Presentation::logCliMessage(
                  formatCliMessage(ActorState.Messages.Dialogue, Dialogue));
              ForbocAI::CLI::Presentation::logCliMessageWhen(
                  Response.bHasAction,
                  formatCliMessage(ActorState.Messages.Action,
                                   Response.Action.Type));
              ForbocAI::CLI::Analysis::logAnalyticalResults(Response,
                                                             ActorState);
              ForbocAI::CLI::Analysis::logPromptTrace(Response, ActorState);
              return CLIOps::CommandResult::Success(Dialogue);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  ActorState.Messages.DecideUsage);
            });
      });
}

/** User Story: As Ghost cognition routing, I need decision and vector roles composed as a focused immutable dispatcher. @fn inline func::Dispatcher<FString, GhostResult> CreateGhostCognitionDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState, int32 First, int32 Second) */
inline func::Dispatcher<FString, GhostResult> CreateGhostCognitionDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState,
    const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState, int32 First,
    int32 Second) {
  return func::createDispatcher<FString, GhostResult>({
      {Roles.GhostDecide,
       [&]() { return DecideGhost(Store, Args, GhostState, ActorState, First,
                                  Second); }},
      {Roles.GhostMemoryStore,
       [&]() { return StoreGhostMemory(Store, Args, GhostState, ActorState,
                                       MemoryState, First, Second); }},
  });
}

} // namespace CLIOps::GhostCommands
