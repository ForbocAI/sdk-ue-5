#pragma once

#include "Features/Protocol/Configuration/ConfigurationAdapters.h"
#include "Features/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the IdentifyActor protocol instruction by serializing actor info
 * and recursing into the next protocol turn.
 * User Story: As protocol instruction dispatch, I need actor identification
 * handled as a pure expression so the instruction ternary stays flat.
 * @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> HandleIdentifyActor(const FNPCProcessResponse &Response, const FString &NpcId, const FString &Input, const FString &RunId, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
HandleIdentifyActor(const FNPCProcessResponse &Response,
                    const FString &NpcId, const FString &Input,
                    const FString &RunId, int32 Turn,
                    const FProtocolHandlerContext &Runtime,
                    std::function<AnyAction(const AnyAction &)> Dispatch,
                    std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  FNPCActorInfo Actor;
  Actor.NpcId = NpcId;
  Actor.Persona = Response.Tape.Persona;
  Actor.bHasStructuredPersona = Response.Tape.bHasStructuredPersona;
  Actor.Data = Response.Tape.NpcState;
  return RunProtocolTurn(NpcId, Input, RunId, Response.Tape,
                         SerializeIdentifyActorResult(Actor), true,
                         Turn + Data.Iteration.Step,
                         Runtime, Dispatch, GetState);
}

} // namespace rtk::detail
