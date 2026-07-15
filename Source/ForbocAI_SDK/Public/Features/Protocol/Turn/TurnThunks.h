#pragma once

#include "Features/API/Endpoints/NPC/NPCApi.h"
#include "Features/Protocol/Instructions/IdentifyActor/IdentifyActorThunks.h"
#include "Features/Protocol/Instructions/QueryVector/QueryVectorThunks.h"
#include "Features/Protocol/Instructions/Decision/DecisionThunks.h"
#include "Features/Protocol/Instructions/Reasoning/ReasoningThunks.h"
#include "Features/Protocol/Instructions/Finalize/FinalizeThunks.h"

namespace rtk::detail {

inline func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolHandlerContext &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState) {
  return Turn >= 12
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId, TEXT("Max turns exceeded"))),
                RejectAsync<FAgentResponse>(
                    TEXT("Protocol loop exceeded max turns")))
             : [&]() -> func::AsyncResult<FAgentResponse> {
    FNPCProcessRequest Request;
    Request.Tape = Tape;
    Request.PreviousResult = LastResult;
    Request.bHasPreviousResult = bHasLastResult;

    return func::AsyncChain::then<FNPCProcessResponse, FAgentResponse>(
               APISlice::Endpoints::postNpcProcess(NpcId, Request)(Dispatch,
                                                                   GetState),
               [NpcId, Input, RunId, Tape, Turn, Runtime, Dispatch,
                GetState](const FNPCProcessResponse &Response)
                   -> func::AsyncResult<FAgentResponse> {
                 const FNPCInstruction &Instruction = Response.Instruction;

                 return Instruction.Type ==
                                ENPCInstructionType::IdentifyActor
                            ? HandleIdentifyActor(Response, NpcId, Input,
                                                  RunId, Turn, Runtime,
                                                  Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::QueryVector
                            ? HandleQueryVector(Response, Instruction, NpcId,
                                                Input, RunId, Turn, Runtime,
                                                Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::Decision
                            ? HandleDecision(Response, NpcId, Input, RunId,
                                             Turn, Runtime, Dispatch, GetState)
                        : Instruction.Type ==
                                  ENPCInstructionType::Reasoning
                            ? HandleReasoning(Response, NpcId, Input, RunId,
                                              Turn, Runtime, Dispatch, GetState)
                        : Instruction.Type == ENPCInstructionType::Finalize
                            ? HandleFinalize(Instruction, NpcId, Input, RunId,
                                             Runtime, Dispatch, GetState)
                            : (Dispatch(
                                   DirectiveSlice::Actions::directiveRunFailed(
                                       RunId,
                                       FString::Printf(
                                           TEXT("Unsupported protocol "
                                                "instruction type: %d"),
                                           static_cast<int32>(
                                               Instruction.Type)))),
                               RejectAsync<FAgentResponse>(FString::Printf(
                                   TEXT("Unsupported protocol instruction "
                                        "type: %d"),
                                   static_cast<int32>(Instruction.Type))));
               })
        .catch_([RunId, Dispatch](std::string Error) {
          Dispatch(DirectiveSlice::Actions::directiveRunFailed(
              RunId, FString(UTF8_TO_TCHAR(Error.c_str()))));
        });
  }();
}

inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolHandlerContext &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const FRuntimeState &()> GetState) {
  return Index >= Instructions.Num()
             ? ResolveAsync(rtk::FEmptyPayload{})
         : !Runtime.StoreMemory
             ? RejectAsync<rtk::FEmptyPayload>(
                   TEXT("API returned memoryStore instructions, but no memory "
                        "engine is configured"))
             : func::AsyncChain::then<FMemoryItem, rtk::FEmptyPayload>(
                   Runtime.StoreMemory(MakeMemoryItem(Instructions[Index]))(
                       Dispatch, GetState),
                   [Instructions, Index, Runtime, Dispatch,
                    GetState](const FMemoryItem &Stored) {
                     return PersistMemoryInstructions(Instructions, Index + 1,
                                                      Runtime, Dispatch,
                                                      GetState);
                   });
}

} // namespace rtk::detail
