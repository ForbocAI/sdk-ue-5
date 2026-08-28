#include "Systems/CLI/NpcCommands/NpcCommandsThunks.h"

#include "Entities/CLI/CLISelectors.h"
#include "Entities/CLI/NPC/CLINPCSelectors.h"
#include "Systems/CLI/NpcCommands/Actor/NpcActorCommandAdapters.h"
#include "Systems/CLI/NpcCommands/Cognition/NpcCognitionCommandAdapters.h"
#include "Systems/CLI/NpcCommands/Soul/NpcSoulCommandAdapters.h"

namespace CLIOps::NpcCommands {

/** User Story: As NPC command routing, I need one role dispatcher composed from named actor, cognition, and soul command concerns. @fn inline func::Dispatcher<FString, NpcResult> CreateNpcDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
inline func::Dispatcher<FString, NpcResult> CreateNpcDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First,
    int32 Second) {
  return func::createDispatcher<FString, NpcResult>({
      {Roles.NpcCreate, [&]() { return CreateNpc(Store, Args, State, First); }},
      {Roles.NpcState,
       [&]() { return PrintNpcState(Store, Args, State, First); }},
      {Roles.NpcUpdate, [&]() { return UpdateNpc(Store, Args, State); }},
      {Roles.NpcProcess, [&]() { return ProcessNpc(Store, Args, State); }},
      {Roles.NpcGenerate,
       [&]() { return GenerateNpcAttribute(Store, Args, State, First, Second); }},
      {Roles.NpcRecall, [&]() { return RecallNpc(Store, Args, State, First); }},
      {Roles.NpcDecide, [&]() { return DecideNpc(Store, Args, State); }},
      {Roles.NpcChat,
       [&]() { return ChatWithNpc(Store, Args, State, First, Second); }},
      {Roles.NpcImport, [&]() { return ImportNpc(Store, Args, State, First); }},
  });
}

} // namespace CLIOps::NpcCommands

namespace CLIOps::CommandRouting {

/** User Story: As a CLI user, I need NPC commands routed through the package root store and named SDK command concerns. @fn RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                            const FString &CommandKey,
                            const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::NPC::FCLINPCState &State =
      ForbocAI::CLI::NPC::selectCliNpc(CLIState);
  return func::dispatch(
      CLIOps::NpcCommands::CreateNpcDispatcher(
          Store, Roles, Args, State, State.Limits.FirstArgumentIndex,
          CLIState.Parsing.SecondTokenIndex),
      CommandKey);
}

} // namespace CLIOps::CommandRouting
