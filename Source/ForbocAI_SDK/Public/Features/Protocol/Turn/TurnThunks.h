#pragma once

#include "Features/API/Endpoints/NPC/NPCApi.h"
#include "Features/Memory/Local/LocalAdapters.h"
#include "Features/Protocol/Configuration/ConfigurationAdapters.h"
#include "Features/Protocol/Instructions/IdentifyActor/IdentifyActorThunks.h"
#include "Features/Protocol/Instructions/QueryVector/QueryVectorThunks.h"
#include "Features/Protocol/Instructions/Decision/DecisionThunks.h"
#include "Features/Protocol/Instructions/Reasoning/ReasoningThunks.h"
#include "Features/Protocol/Instructions/Finalize/FinalizeThunks.h"

namespace rtk::detail {

/** User Story: As protocol execution, I need turn recursion parameterized by package state so feature headers remain independent of the root store. @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> RunProtocolTurn(const FString &NpcId, const FString &Input, const FString &RunId, const FNPCProcessTape &Tape, const FString &LastResult, bool bHasLastResult, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState) */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolHandlerContext &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  return Turn >= Data.Limits.MaxTurns
             ? (Dispatch(DirectiveSlice::Actions::directiveRunFailed(
                    RunId, Data.Errors.MaxTurnsExceeded)),
                RejectAsync<FAgentResponse>(Data.Errors.MaxTurnsExceeded))
             : [&]() -> func::AsyncResult<FAgentResponse> {
    FNPCProcessRequest Request;
    Request.Tape = Tape;
    Request.PreviousResult = LastResult;
    Request.bHasPreviousResult = bHasLastResult;

    return func::AsyncChain::then<FNPCProcessResponse, FAgentResponse>(
               APISlice::Endpoints::postNpcProcess(NpcId, Request)(Dispatch,
                                                                   GetState),
               [NpcId, Input, RunId, Tape, Turn, Runtime, Dispatch, GetState,
                UnsupportedInstruction = Data.Errors.UnsupportedInstruction](
                   const FNPCProcessResponse &Response)
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
                            : [&]() {
                                const FString Error =
                                    UnsupportedInstruction +
                                    LexToString(
                                        static_cast<int32>(Instruction.Type));
                                Dispatch(DirectiveSlice::Actions::
                                             directiveRunFailed(RunId, Error));
                                return RejectAsync<FAgentResponse>(Error);
                              }();
               })
        .catch_([RunId, Dispatch](std::string Error) {
          Dispatch(DirectiveSlice::Actions::directiveRunFailed(
              RunId, FString(UTF8_TO_TCHAR(Error.c_str()))));
        });
  }();
}

/** User Story: As protocol persistence, I need memory effects parameterized by the package state so feature headers never import the root store. @fn template <typename RuntimeState> inline func::AsyncResult<rtk::FEmptyPayload> PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions, int32 Index, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState) */
template <typename RuntimeState>
inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolHandlerContext &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  return Index >= Instructions.Num()
             ? ResolveAsync(rtk::FEmptyPayload{})
         : !Runtime.StoreMemory
             ? RejectAsync<rtk::FEmptyPayload>(Data.Errors.MissingMemoryStore)
             : func::AsyncChain::then<FMemoryItem, rtk::FEmptyPayload>(
                   Runtime.StoreMemory(
                       MemoryLocalAdapters::createMemoryItemAdapter(
                           Instructions[Index].Text, Instructions[Index].Type,
                           Instructions[Index].Importance))(
                       Dispatch, GetState),
                   [Instructions, Index, Runtime, Dispatch, GetState,
                    Step = Data.Iteration.Step](const FMemoryItem &Stored) {
                     return PersistMemoryInstructions(
                         Instructions, Index + Step, Runtime,
                         Dispatch, GetState);
                   });
}

} // namespace rtk::detail
