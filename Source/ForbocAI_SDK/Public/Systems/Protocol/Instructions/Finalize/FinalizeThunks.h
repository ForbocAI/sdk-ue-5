#pragma once

#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/NPC/NPCActions.h"
#include "Systems/Protocol/Configuration/ProtocolConfigurationAdapters.h"
#include "Systems/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the Finalize protocol instruction by validating the verdict,
 * persisting memory, and applying state transforms.
 * User Story: As protocol instruction dispatch, I need finalization handled
 * as a pure expression so the instruction ternary stays flat.
 * @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> HandleFinalize(const FNPCProcessResponse &Response, const FString &NpcId, const FString &Input, const FString &RunId, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
HandleFinalize(const FNPCProcessResponse &Response,
               const FString &NpcId, const FString &Input,
               const FString &RunId,
               const FProtocolHandlerContext &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  const FNPCInstruction &Instruction = Response.Instruction;
  FVerdictResponse Verdict;
  Verdict.bValid = Instruction.bValid;
  Verdict.Signature = Instruction.Signature;
  Verdict.storeMemory = Instruction.storeMemory;
  Verdict.StateDelta = Instruction.StateTransform;
  Verdict.Dialogue = Instruction.Dialogue;
  Verdict.bHasAction = Instruction.bHasAction;
  Verdict.Action = Instruction.Action;
  Dispatch(DirectiveSlice::Actions::verdictValidated(RunId, Verdict));

  return !Instruction.bValid
             ? (Dispatch(NPCActions::blockAction(
                    NpcId, Instruction.Dialogue.IsEmpty()
                               ? Data.Errors.ValidationFailed
                               : Instruction.Dialogue)),
                ResolveAsync(BuildAgentResponse(Instruction, Response.Tape)))
             : func::AsyncChain::then<rtk::FEmptyPayload, FAgentResponse>(
                   PersistMemoryInstructions(
                       Instruction.storeMemory, Data.Iteration.InitialIndex,
                       Runtime, Dispatch, GetState),
                   [NpcId, Input, Instruction, Tape = Response.Tape, Dispatch, Data,
                    GetState](const rtk::FEmptyPayload &) {
                     HasStatePayload(Instruction.StateTransform)
                         ? (Dispatch(NPCActions::updateNPCState(
                                NpcId, Instruction.StateTransform)),
                            void())
                         : void();

                     Instruction.bHasAction
                         ? (Dispatch(NPCActions::actionReceived(
                                NpcId, Instruction.Action)),
                            void())
                         : void();

                     Dispatch(NPCActions::addToHistory(
                         NpcId, Data.Roles.User, Input));
                     Dispatch(NPCActions::addToHistory(
                         NpcId, Data.Roles.Assistant, Instruction.Dialogue));

                     return ResolveAsync(BuildAgentResponse(Instruction, Tape));
                   });
}

} // namespace rtk::detail
