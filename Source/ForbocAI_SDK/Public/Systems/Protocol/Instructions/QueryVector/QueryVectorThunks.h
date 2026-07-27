#pragma once

#include "Entities/Directive/DirectiveSlice.h"
#include "Systems/Protocol/Configuration/ProtocolConfigurationAdapters.h"
#include "Systems/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the QueryVector protocol instruction by dispatching a memory recall
 * and building the recalled-memory tape for the next turn.
 * User Story: As protocol instruction dispatch, I need vector queries handled
 * as a pure expression so the instruction ternary stays flat.
 * @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> HandleQueryVector(const FNPCProcessResponse &Response, const FNPCInstruction &Instruction, const FString &NpcId, const FString &Input, const FString &RunId, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
HandleQueryVector(const FNPCProcessResponse &Response,
                  const FNPCInstruction &Instruction,
                  const FString &NpcId, const FString &Input,
                  const FString &RunId, int32 Turn,
                  const FProtocolHandlerContext &Runtime,
                  std::function<AnyAction(const AnyAction &)> Dispatch,
                  std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  return !Runtime.HasMemory()
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId, Data.Errors.MissingVectorMemory)),
                RejectAsync<FAgentResponse>(Data.Errors.MissingVectorMemory))
             : [&]() -> func::AsyncResult<FAgentResponse> {
    FDirectiveResponse Directive;
    Directive.recallMemory = TypeFactory::MemoryRecallInstruction(
        Instruction.Query, Instruction.Limit, Instruction.Threshold);
    Dispatch(
        DirectiveSlice::Actions::directiveReceived(RunId, Directive));

    FMemoryRecallRequest RecallRequest;
    RecallRequest.Query = Instruction.Query;
    RecallRequest.Limit = Instruction.Limit;
    RecallRequest.Threshold = Instruction.Threshold;

    return func::AsyncChain::then<TArray<FMemoryItem>, FAgentResponse>(
        Runtime.RecallMemory(RecallRequest)(Dispatch, GetState),
        [NpcId, Input, RunId, Response, Turn, Dispatch, GetState, Runtime,
         Step = Data.Iteration.Step](const TArray<FMemoryItem> &Memories) {
          FNPCProcessTape NextTape = Response.Tape;
          NextTape.Memories = MemoryItemsToRecalled(Memories);
          NextTape.bVectorQueried = true;
          return RunProtocolTurn(
              NpcId, Input, RunId, NextTape,
              SerializeQueryVectorResult(Memories), true, Turn + Step,
              Runtime, Dispatch, GetState);
        });
  }();
}

} // namespace rtk::detail
