#include "Systems/CLI/GhostCommands/GhostCommandsThunks.h"

#include "Entities/CLI/CLISelectors.h"
#include "Entities/CLI/Ghost/CLIGhostSelectors.h"
#include "Entities/CLI/Memory/CLIMemorySelectors.h"
#include "Entities/CLI/NPC/CLINPCSelectors.h"
#include "Systems/CLI/GhostCommands/Actor/GhostCommandsActorAdapters.h"
#include "Systems/CLI/GhostCommands/Cognition/GhostCommandsCognitionAdapters.h"
#include "Systems/CLI/GhostCommands/Lifecycle/LifecycleAdapters.h"

namespace CLIOps::GhostCommands {

/**
 * User Story: As Ghost command routing, I need actor, cognition, and lifecycle dispatchers tried in a deterministic functional chain.
 * @fn inline func::Maybe<GhostResult> DispatchGhostCommandGroups( rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const FString &CommandKey, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState, const ForbocAI::CLI::FCLIParsingSettings &Parsing)
 */
inline func::Maybe<GhostResult> DispatchGhostCommandGroups(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const FString &CommandKey, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState,
    const ForbocAI::CLI::Memory::FCLIMemoryState &MemoryState,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing) {
  const int32 First = Parsing.FirstTokenIndex;
  const int32 Second = Parsing.SecondTokenIndex;
  const func::Maybe<GhostResult> Actor = func::dispatch(
      CreateGhostActorDispatcher(Store, Roles, Args, GhostState, ActorState,
                                 First, Second, Parsing.NextIndexOffset),
      CommandKey);
  return func::match(
      Actor, [](const GhostResult &Result) { return func::just(Result); },
      [&]() {
        const func::Maybe<GhostResult> Cognition = func::dispatch(
            CreateGhostCognitionDispatcher(Store, Roles, Args, GhostState,
                                            ActorState, MemoryState, First,
                                            Second),
            CommandKey);
        return func::match(
            Cognition,
            [](const GhostResult &Result) { return func::just(Result); },
            [&]() {
              return func::dispatch(
                  CreateGhostLifecycleDispatcher(Store, Roles, Args,
                                                 GhostState, First, Second,
                                                 Parsing.NextIndexOffset),
                  CommandKey);
            });
      });
}

} // namespace CLIOps::GhostCommands

namespace CLIOps::CommandRouting {

/** User Story: As a CLI user, I need every Ghost command routed through the package root store and Ghost-only SDK operations. @fn RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                              const FString &CommandKey,
                              const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  return CLIOps::GhostCommands::DispatchGhostCommandGroups(
      Store, ForbocAI::CLI::selectCliCommandRoles(CLIState), CommandKey,
      Args, ForbocAI::CLI::Ghost::selectCliGhost(CLIState),
      ForbocAI::CLI::NPC::selectCliNpc(CLIState),
      ForbocAI::CLI::Memory::selectCliMemory(CLIState), CLIState.Parsing);
}

} // namespace CLIOps::CommandRouting
