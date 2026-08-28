#pragma once

#include "Systems/API/Endpoints/NPC/Generate/GenerateAdapters.h"
#include "Systems/CLI/Analysis/CLIAnalysisAdapters.h"
#include "Systems/CLI/CommandResult/CommandResultAdapters.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/CLI/NPC/Decision/NPCDecisionAdapters.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Systems/Protocol/Process/ProcessAdapters.h"

namespace CLIOps::NpcCommands {

using NpcResult = CLIOps::CommandResult::Result;

/** User Story: As NPC cognition presentation, I need one API response rendered consistently without interpreting its analytical fields. @fn inline FString PresentNpcResponse(const FAgentResponse &Response, const ForbocAI::CLI::NPC::FCLINPCState &State, const FString &DialogueFormat, const FString &ActionFormat, bool bTracePrompt) */
inline FString PresentNpcResponse(
    const FAgentResponse &Response,
    const ForbocAI::CLI::NPC::FCLINPCState &State,
    const FString &DialogueFormat, const FString &ActionFormat,
    bool bTracePrompt) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const FString Dialogue =
      ForbocAI::CLI::NPC::selectDialogue(Response, State);
  ForbocAI::CLI::Presentation::logCliMessage(
      formatCliMessage(DialogueFormat, Dialogue));
  ForbocAI::CLI::Presentation::logCliMessageWhen(
      Response.bHasAction,
      formatCliMessage(ActionFormat, Response.Action.Type));
  ForbocAI::CLI::Analysis::logAnalyticalResults(Response, State);
  bTracePrompt ? ForbocAI::CLI::Analysis::logPromptTrace(Response, State)
               : void();
  return Dialogue;
}

/** User Story: As normal NPC processing, I need CLI process arguments projected into one normal SDK process operation. @fn inline NpcResult ProcessNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State) */
inline NpcResult ProcessNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const FAgentResponse Response = Ops::processNpc(
            Store, ForbocAI::CLI::NPC::toProtocolProcessInput(
                       ForbocAI::CLI::NPC::decodeNpcProcessCommand(Args,
                                                                   State)));
        return CLIOps::CommandResult::Success(PresentNpcResponse(
            Response, State, State.Messages.Dialogue, State.Messages.Action,
            true));
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.ProcessUsage);
      });
}

/** User Story: As NPC persona composition, I need one authored attribute request delegated to the normal SDK generation endpoint. @fn inline NpcResult GenerateNpcAttribute(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
inline NpcResult GenerateNpcAttribute(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First,
    int32 Second) {
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() > First,
      [&]() {
        const FString Context =
            Args.Num() > Second
                ? ForbocAI::CLI::NPC::joinNpcArguments(
                      Args, Second, State.Syntax.MessageSeparator)
                : FString();
        const FNpcAttributeGenerateResponse Response =
            AsyncAdapters::waitForResult(
                Ops::generateNpcAttribute(Store, Args[First], Context));
        ForbocAI::CLI::Presentation::logCliMessage(
            APISlice::Detail::EncodeNpcAttributeGenerateResponse(Response));
        return CLIOps::CommandResult::Success(Response.Value);
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.GenerateUsage);
      });
}

/** User Story: As normal NPC constrained choice, I need CLI options projected into one normal SDK decision operation. @fn inline NpcResult DecideNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State) */
inline NpcResult DecideNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State) {
  return CLIOps::CommandRouting::matchCondition<NpcResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const FAgentResponse Response = Ops::decideNpc(
            Store, ForbocAI::CLI::NPC::toProtocolProcessInput(
                       ForbocAI::CLI::NPC::decodeNpcDecisionCommand(Args,
                                                                    State)));
        return CLIOps::CommandResult::Success(PresentNpcResponse(
            Response, State, State.Messages.Dialogue, State.Messages.Action,
            true));
      },
      [&]() {
        return CLIOps::CommandResult::Failure(State.Messages.DecideUsage);
      });
}

/** User Story: As normal NPC chat, I need one message projected into the same normal SDK process operation used by other NPC commands. @fn inline NpcResult ChatWithNpc(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First, int32 Second) */
inline NpcResult ChatWithNpc(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::NPC::FCLINPCState &State, int32 First,
    int32 Second) {
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
        const FAgentResponse Response = Ops::processNpc(
            Store, ProtocolProcess::ProcessInput(Args[First], Message));
        return CLIOps::CommandResult::Success(PresentNpcResponse(
            Response, State, State.Messages.ChatNpc,
            State.Messages.ChatAction, false));
      },
      [&]() {
        return CLIOps::CommandResult::Failure(
            Args.Num() == State.Limits.EmptyArgumentCount
                ? State.Messages.ChatMissingId
                : State.Messages.ChatUsage);
      });
}

} // namespace CLIOps::NpcCommands
