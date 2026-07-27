#pragma once

#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Systems/Protocol/Configuration/ProtocolConfigurationAdapters.h"
#include "Systems/Protocol/Requests/RequestsAdapters.h"
#include "Systems/Protocol/Handlers/HandlersThunks.h"
#include "Systems/Protocol/Turn/TurnThunks.h"

namespace rtk {

/**
 * Protocol thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 * @fn template <typename RuntimeState = FRuntimeState> inline ThunkAction<FAgentResponse, RuntimeState> processNPC(const FString &NpcId, const FString &Input, const FString &ContextJson, const FString &Persona, const FAgentState &InitialState, const FProtocolHandlerContext &Runtime)
 */

template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processNPC(const FString &NpcId, const FString &Input,
           const FString &ContextJson, const FString &Persona,
           const FAgentState &InitialState,
           const FProtocolHandlerContext &Runtime) {
  return [NpcId, Input, ContextJson, Persona, InitialState, Runtime](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const RuntimeState &()> GetState)
             -> func::AsyncResult<FAgentResponse> {
    const auto &Data = ProtocolConfiguration::protocolData();
    const auto ExistingNpc = NPCSelectors::selectNPCById(GetState().NPCs, NpcId);
    const bool bHasExplicitState =
        !InitialState.JsonData.IsEmpty() &&
        InitialState.JsonData != Data.Text.EmptyObject;

    const FString ResolvedPersona =
        ExistingNpc.hasValue && Persona.IsEmpty()
            ? ExistingNpc.value.Persona
            : Persona;

    const FAgentState CurrentState =
        ExistingNpc.hasValue && !bHasExplicitState
            ? ExistingNpc.value.State
            : InitialState;

    return [&]() -> func::AsyncResult<FAgentResponse> {
      !ExistingNpc.hasValue
          ? [&]() {
              FNPCInternalState Info;
              Info.Id = NpcId;
              Info.Persona = ResolvedPersona;
              Info.State = InitialState;
              Dispatch(NPCActions::setNPCInfo(Info));
            }()
          : (NPCSelectors::selectActiveNpcId(GetState().NPCs) != NpcId
                 ? (Dispatch(NPCActions::setActiveNPC(NpcId)), void())
                 : void());

      const FString RunId =
          NpcId + Data.Formats.RunIdSeparator +
          LexToString(FDateTime::UtcNow().ToUnixTimestamp());
      Dispatch(
          DirectiveSlice::Actions::directiveRunStarted(RunId, NpcId, Input));

      FNPCProcessTape Tape = ProtocolRequests::ProcessTape(
          Input, ContextJson, CurrentState, ResolvedPersona);
      Tape.Memories.Empty();
      Tape.bVectorQueried = false;
      Tape.bHasVectorQueried = true;

      return detail::RunProtocolTurn(
          NpcId, Input, RunId, Tape, Data.Text.Empty, false,
          Data.Iteration.InitialIndex, Runtime, Dispatch, GetState);
    }();
  };
}

} // namespace rtk
