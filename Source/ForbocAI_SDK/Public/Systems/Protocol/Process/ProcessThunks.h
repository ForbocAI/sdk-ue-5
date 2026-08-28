#pragma once

#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Systems/Protocol/Configuration/ProtocolConfigurationAdapters.h"
#include "Systems/Protocol/Requests/RequestsAdapters.h"
#include "Systems/Protocol/Handlers/HandlersThunks.h"
#include "Systems/Protocol/Process/ProcessAdapters.h"
#include "Systems/Protocol/Turn/TurnThunks.h"
#include "Systems/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"

namespace rtk {

/**
 * Protocol thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 * @fn template <typename RuntimeState = FRuntimeState> inline ThunkAction<FAgentResponse, RuntimeState> processWithRoute(const FProtocolProcessInput &Input, const FProtocolHandlerContext &Runtime)
 */

template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processWithRoute(const FProtocolProcessInput &Input,
                 const FProtocolHandlerContext &Runtime) {
  return [Input, Runtime](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const RuntimeState &()> GetState)
             -> func::AsyncResult<FAgentResponse> {
    const auto &Data = ProtocolConfiguration::protocolData();
    const auto ExistingNpc =
        NPCSelectors::selectNPCById(GetState().NPCs, Input.NpcId);
    const bool bHasExplicitState =
        !Input.InitialState.JsonData.IsEmpty() &&
        Input.InitialState.JsonData !=
            APISlice::Endpoints::Configuration::endpointData()
                .Payloads.EmptyObject;

    const FString ResolvedPersona =
        ExistingNpc.hasValue && Input.Persona.IsEmpty()
            ? ExistingNpc.value.Persona
            : Input.Persona;

    const FAgentState CurrentState =
        ExistingNpc.hasValue && !bHasExplicitState
            ? ExistingNpc.value.State
            : Input.InitialState;

    return [&]() -> func::AsyncResult<FAgentResponse> {
      !ExistingNpc.hasValue
          ? [&]() {
              FNPCInternalState Info;
              Info.Id = Input.NpcId;
              Info.Persona = ResolvedPersona;
              Info.State = CurrentState;
              Dispatch(NPCActions::setNPCInfo(Info));
            }()
          : (NPCSelectors::selectActiveNpcId(GetState().NPCs) != Input.NpcId
                 ? (Dispatch(NPCActions::setActiveNPC(Input.NpcId)), void())
                 : void());

      const FString RunId =
          Input.NpcId + Data.Formats.RunIdSeparator +
          LexToString(FDateTime::UtcNow().ToUnixTimestamp());
      Dispatch(
          DirectiveSlice::Actions::directiveRunStarted(
              RunId, Input.NpcId, Input.Observation));

      FProtocolProcessInput ResolvedInput = Input;
      ResolvedInput.Persona = ResolvedPersona;
      FNPCProcessTape Tape =
          ProtocolRequests::ProcessTape(ResolvedInput, CurrentState);
      Tape.Memories.Empty();
      Tape.bVectorQueried = false;
      Tape.bHasVectorQueried = true;

      return detail::RunProtocolTurn(
          Input.NpcId, Input.Observation, RunId, Tape, FString(), false,
          Data.Iteration.InitialIndex, Runtime, Dispatch, GetState);
    }();
  };
}

/** User Story: As ordinary NPC cognition, I need the shared tape interpreter permanently bound to the NPC endpoint family. @fn template <typename RuntimeState = FRuntimeState> inline ThunkAction<FAgentResponse, RuntimeState> processNPC(const FProtocolProcessInput &Input, const FProtocolHandlerContext &Runtime) */
template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processNPC(const FProtocolProcessInput &Input,
           const FProtocolHandlerContext &Runtime) {
  return processWithRoute<RuntimeState>(
      Input, WithNpcProcessRoute(Input.NpcId, Runtime));
}

/** User Story: As Ghost cognition, I need the shared tape interpreter permanently bound to the active Ghost endpoint family. @fn template <typename RuntimeState = FRuntimeState> inline ThunkAction<FAgentResponse, RuntimeState> processGhost(const FGhostProcessInput &Input, const FProtocolHandlerContext &Runtime) */
template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processGhost(const FGhostProcessInput &Input,
             const FProtocolHandlerContext &Runtime) {
  return processWithRoute<RuntimeState>(
      Input.Process, WithGhostProcessRoute(Input.SessionId, Runtime));
}

} // namespace rtk
