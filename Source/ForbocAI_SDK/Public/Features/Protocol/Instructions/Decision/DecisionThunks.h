#pragma once

#include "Features/Protocol/Configuration/ProtocolConfigurationAdapters.h"
#include "Features/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the Decision protocol instruction by validating and echoing the
 * API-owned decision intent without applying SDK policy.
 * User Story: As a protocol instructions decision consumer, I need to invoke handle decision through a stable signature so the protocol instructions decision workflow remains explicit and composable.
 * @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> HandleDecision(const FNPCProcessResponse &Response, const FString &NpcId, const FString &Input, const FString &RunId, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
HandleDecision(const FNPCProcessResponse &Response,
               const FString &NpcId, const FString &Input,
               const FString &RunId, int32 Turn,
               const FProtocolHandlerContext &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  return !Response.Tape.bDecisionCompleted
             ? RejectAsync<FAgentResponse>(
                   Data.Errors.MissingApiDecisionIntent)
             : RunProtocolTurn(
                   NpcId, Input, RunId, Response.Tape,
                   SerializeDecisionResult(
                       Response.Tape.DecisionIntent.Goal,
                       Response.Tape.DecisionIntent.ActionType,
                       Response.Tape.DecisionIntent.Target),
                   true, Turn + Data.Iteration.Step, Runtime, Dispatch,
                   GetState);
}

} // namespace rtk::detail
