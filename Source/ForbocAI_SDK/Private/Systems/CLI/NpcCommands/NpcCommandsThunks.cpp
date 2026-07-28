#include "Systems/CLI/NpcCommands/NpcCommandsThunks.h"
#include "Core/JsonInterop.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Entities/CLI/NPC/CLINPCSelectors.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace {

using NpcResult = CLIOps::CommandRouting::Result;

/** User Story: As a CLI command-routing consumer, I need to invoke npc success through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result NpcSuccess(const FString &Message) */
CLIOps::CommandRouting::Result NpcSuccess(const FString &Message) {
  return CLIOps::CommandRouting::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing consumer, I need to invoke npc failure through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result NpcFailure(const FString &Message) */
CLIOps::CommandRouting::Result NpcFailure(const FString &Message) {
  return CLIOps::CommandRouting::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke create npc through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult CreateNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
NpcResult CreateNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                    const TArray<FString> &Args,
                    const ForbocAI::CLI::NPC::FCLINPCState &State,
                    int32 First) {
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
        return NpcSuccess(Npc.Id);
      },
      [&]() { return NpcFailure(State.Messages.CreateUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke process npc through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult ProcessNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
NpcResult ProcessNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                     const TArray<FString> &Args,
                     const ForbocAI::CLI::NPC::FCLINPCState &State,
                     int32 First, int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const FString Observation = ForbocAI::CLI::NPC::joinNpcArguments(
            Args, Second, State.Syntax.MessageSeparator);
        const FAgentResponse Response =
            Ops::processNpc(Store, Args[First], Observation);
        const FString Dialogue =
            ForbocAI::CLI::NPC::selectDialogue(Response, State);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Dialogue, Dialogue));
        ForbocAI::CLI::Presentation::logCliMessageWhen(
            !Response.Action.Type.IsEmpty(),
            formatCliMessage(State.Messages.Action, Response.Action.Type));
        return NpcSuccess(Dialogue);
      },
      [&]() { return NpcFailure(State.Messages.ProcessUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need one SLM-generated NPC attribute emitted as structured JSON so personas compose one round trip at a time. @fn NpcResult GenerateNpcAttribute(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
NpcResult GenerateNpcAttribute(rtk::EnhancedStore<FRuntimeState> &Store,
                               const TArray<FString> &Args,
                               const ForbocAI::CLI::NPC::FCLINPCState &State,
                               int32 First, int32 Second) {
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() > First,
      [&]() {
        const FString Attribute = Args[First];
        const FString Context =
            Args.Num() > Second
                ? ForbocAI::CLI::NPC::joinNpcArguments(
                      Args, Second, State.Syntax.MessageSeparator)
                : FString();
        const FNpcAttributeGenerateResponse Response =
            AsyncAdapters::waitForResult(
                Ops::generateNpcAttribute(Store, Attribute, Context));
        const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        Root->SetStringField(TEXT("attribute"), Response.Attribute);
        Root->SetStringField(TEXT("value"), Response.Value);
        ForbocAI::CLI::Presentation::logCliMessage(
            JsonInterop::ToJsonString(Root));
        return NpcSuccess(Response.Value);
      },
      [&]() { return NpcFailure(State.Messages.GenerateUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need the decide-only npc turn routed so the micro-game composes memory recall and store around it as separate commands. @fn NpcResult DecideNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
NpcResult DecideNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                    const TArray<FString> &Args,
                    const ForbocAI::CLI::NPC::FCLINPCState &State,
                    int32 First, int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const FString Observation = ForbocAI::CLI::NPC::joinNpcArguments(
            Args, Second, State.Syntax.MessageSeparator);
        const FAgentResponse Response =
            Ops::decideNpc(Store, Args[First], Observation);
        const FString Dialogue =
            ForbocAI::CLI::NPC::selectDialogue(Response, State);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Dialogue, Dialogue));
        ForbocAI::CLI::Presentation::logCliMessageWhen(
            !Response.Action.Type.IsEmpty(),
            formatCliMessage(State.Messages.Action, Response.Action.Type));
        return NpcSuccess(Dialogue);
      },
      [&]() { return NpcFailure(State.Messages.DecideUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke print npc state through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult PrintNpcState(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
NpcResult PrintNpcState(rtk::EnhancedStore<FRuntimeState> &Store,
                        const TArray<FString> &Args,
                        const ForbocAI::CLI::NPC::FCLINPCState &State,
                        int32 First) {
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
        return NpcSuccess(State.Messages.StatePrinted);
      },
      [&Args, &State, First, bHasExplicitNpc]() {
        return NpcFailure(CLIOps::CommandRouting::matchCondition<FString>(
            bHasExplicitNpc,
            [&]() {
              return formatCliMessage(State.Messages.NotFound, Args[First]);
            },
            [&]() { return State.Messages.NoActive; }));
      });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke update npc through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult UpdateNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State) */
NpcResult UpdateNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                    const TArray<FString> &Args,
                    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const func::Maybe<FNPCInternalState> Active =
      Ops::getActiveNpc(Store);
  const func::Maybe<FString> ActiveId = func::fmap(
      Active, [](const FNPCInternalState &Npc) { return Npc.Id; });
  const func::Maybe<ForbocAI::CLI::NPC::FCLINPCUpdate> Update =
      ForbocAI::CLI::NPC::decodeNpcUpdate(Args, ActiveId, State);
  return func::match(
      Update,
      [&Store, &State](const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded) {
        return CLIOps::CommandRouting::matchCondition<NpcResult>(
            Ops::getNpc(Store, Decoded.NpcId).hasValue,
            [&]() {
              Ops::updateNpc(Store, Decoded.NpcId, Decoded.Delta);
              ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
                  State.Messages.Updated, Decoded.NpcId,
                  Decoded.Delta.JsonData));
              return NpcSuccess(State.Messages.UpdateDone);
            },
            [&]() {
              return NpcFailure(
                  formatCliMessage(State.Messages.NotFound, Decoded.NpcId));
            });
      },
      [&State]() { return NpcFailure(State.Messages.UpdateUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke import npc through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult ImportNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First) */
NpcResult ImportNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                    const TArray<FString> &Args,
                    const ForbocAI::CLI::NPC::FCLINPCState &State,
                    int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        const FImportedNpc Npc = Ops::importNpcFromSoul(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Imported, Npc.NpcId));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Persona, Npc.Persona));
        return NpcSuccess(State.Messages.ImportDone);
      },
      [&]() { return NpcFailure(State.Messages.ImportUsage); });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke chat with npc through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn NpcResult ChatWithNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
NpcResult ChatWithNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                      const TArray<FString> &Args,
                      const ForbocAI::CLI::NPC::FCLINPCState &State,
                      int32 First, int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const FString Message = ForbocAI::CLI::NPC::joinNpcArguments(
            Args, Second, State.Syntax.MessageSeparator);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.ChatHeader, Args[First]));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.ChatUser, Message));
        const FAgentResponse Response =
            Ops::processNpc(Store, Args[First], Message);
        const FString Dialogue =
            ForbocAI::CLI::NPC::selectDialogue(Response, State);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.ChatNpc, Dialogue));
        ForbocAI::CLI::Presentation::logCliMessageWhen(
            !Response.Action.Type.IsEmpty(),
            formatCliMessage(State.Messages.ChatAction, Response.Action.Type));
        return NpcSuccess(Dialogue);
      },
      [&]() {
        return NpcFailure(CLIOps::CommandRouting::matchCondition<FString>(
            Args.Num() == State.Limits.EmptyArgumentCount,
            [&]() { return State.Messages.ChatMissingId; },
            [&]() { return State.Messages.ChatUsage; }));
      });
}

/** User Story: As a CLI command-routing npc consumer, I need to invoke create npc dispatcher through a stable signature so the CLI command-routing npc workflow remains explicit and composable. @fn func::Dispatcher<FString, NpcResult> CreateNpcDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
func::Dispatcher<FString, NpcResult>
CreateNpcDispatcher(rtk::EnhancedStore<FRuntimeState> &Store,
                    const ForbocAI::CLI::FCLICommandRoles &Roles,
                    const TArray<FString> &Args,
                    const ForbocAI::CLI::NPC::FCLINPCState &State,
                    int32 First, int32 Second) {
  return func::createDispatcher<FString, NpcResult>({
      {Roles.NpcCreate, [&]() { return CreateNpc(Store, Args, State, First); }},
      {Roles.NpcProcess,
       [&]() { return ProcessNpc(Store, Args, State, First, Second); }},
      {Roles.NpcGenerate,
       [&]() { return GenerateNpcAttribute(Store, Args, State, First, Second); }},
      {Roles.NpcDecide,
       [&]() { return DecideNpc(Store, Args, State, First, Second); }},
      {Roles.NpcState,
       [&]() { return PrintNpcState(Store, Args, State, First); }},
      {Roles.NpcUpdate, [&]() { return UpdateNpc(Store, Args, State); }},
      {Roles.NpcImport, [&]() { return ImportNpc(Store, Args, State, First); }},
      {Roles.NpcChat,
       [&]() { return ChatWithNpc(Store, Args, State, First, Second); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI command-routing consumer, I need to invoke handle npc through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FString &CommandKey,
                        const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::NPC::FCLINPCState &State =
      ForbocAI::CLI::NPC::selectCliNpc(CLIState);
  const int32 First = State.Limits.FirstArgumentIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;
  return func::dispatch(
      CreateNpcDispatcher(Store, Roles, Args, State, First, Second),
      CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
