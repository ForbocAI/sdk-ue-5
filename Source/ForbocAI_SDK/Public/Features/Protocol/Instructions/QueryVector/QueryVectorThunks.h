#pragma once

#include "Features/Directive/DirectiveSlice.h"
#include "Features/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the QueryVector protocol instruction by dispatching a memory recall
 * and building the recalled-memory tape for the next turn.
 * User Story: As protocol instruction dispatch, I need vector queries handled
 * as a pure expression so the instruction ternary stays flat.
 */
inline func::AsyncResult<FAgentResponse>
HandleQueryVector(const FNPCProcessResponse &Response,
                  const FNPCInstruction &Instruction,
                  const FString &NpcId, const FString &Input,
                  const FString &RunId, int32 Turn,
                  const FProtocolHandlerContext &Runtime,
                  std::function<AnyAction(const AnyAction &)> Dispatch,
                  std::function<const FRuntimeState &()> GetState) {
  return !Runtime.HasMemory()
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId,
                    TEXT("API requested memory recall, but no memory engine "
                         "is configured"))),
                RejectAsync<FAgentResponse>(
                    TEXT("API requested memory recall, but no memory engine "
                         "is configured")))
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
        [NpcId, Input, RunId, Response, Turn, Dispatch, GetState,
         Runtime](const TArray<FMemoryItem> &Memories) {
          FNPCProcessTape NextTape = Response.Tape;
          NextTape.Memories =
              PopulateRecalledMemoriesRecursive(Memories, 0, TArray<FRecalledMemory>());
          NextTape.bVectorQueried = true;
          return RunProtocolTurn(
              NpcId, Input, RunId, NextTape,
              SerializeQueryVectorResult(Memories), true, Turn + 1,
              Runtime, Dispatch, GetState);
        });
  }();
}

} // namespace rtk::detail
