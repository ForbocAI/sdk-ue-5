#pragma once

#include "Features/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the Reasoning protocol instruction.
 * User Story: As protocol instruction dispatch, I need the Reasoning step
 * acknowledged so the multi-round loop can advance past it without stalling.
 *
 * Architectural note (2026-04-28): SLM inference is now API-hosted. The API
 * runs the model itself and populates Tape.ReasoningText / Tape.ResponseText
 * before returning. The SDK's only remaining responsibility for a Reasoning
 * step is to mark the tape completed and continue. SDK-local inference is
 * deliberately not invoked here.
 */
inline func::AsyncResult<FAgentResponse>
HandleReasoning(const FNPCProcessResponse &Response,
                const FString &NpcId, const FString &Input,
                const FString &RunId, int32 Turn,
                const FProtocolHandlerContext &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState) {
  FNPCProcessTape NextTape = Response.Tape;
  NextTape.bReasoningCompleted = true;

  return RunProtocolTurn(
      NpcId, Input, RunId, NextTape,
      SerializeReasoningResult(NextTape.ReasoningOutput.ReasoningText, NextTape.ReasoningOutput.ResponseText),
      true, Turn + 1, Runtime, Dispatch, GetState);
}

} // namespace rtk::detail
