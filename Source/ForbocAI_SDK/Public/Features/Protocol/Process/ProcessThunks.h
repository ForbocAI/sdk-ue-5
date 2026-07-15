#pragma once

#include "Features/Directive/DirectiveSlice.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/Requests/RequestsAdapters.h"
#include "Features/Protocol/Handlers/HandlersThunks.h"
#include "Features/Protocol/Turn/TurnThunks.h"

namespace rtk {

/**
 * Protocol thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename RuntimeState = FRuntimeState>
inline ThunkAction<FAgentResponse, RuntimeState>
processNPC(const FString &NpcId, const FString &Input = TEXT(""),
           const FString &ContextJson = TEXT("{}"),
           const FString &Persona = TEXT(""),
           const FAgentState &InitialState = FAgentState(),
           const FProtocolHandlerContext &Runtime = FProtocolHandlerContext()) {
  return [NpcId, Input, ContextJson, Persona, InitialState, Runtime](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const RuntimeState &()> GetState)
             -> func::AsyncResult<FAgentResponse> {
    const auto ExistingNpc = NPCSelectors::selectNPCById(GetState().NPCs, NpcId);
    const bool bHasExplicitState =
        !InitialState.JsonData.IsEmpty() && InitialState.JsonData != TEXT("{}");

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

      const FString RunId = FString::Printf(
          TEXT("%s:%lld"), *NpcId, FDateTime::UtcNow().ToUnixTimestamp());
      Dispatch(
          DirectiveSlice::Actions::directiveRunStarted(RunId, NpcId, Input));

      FNPCProcessTape Tape = ProtocolRequests::ProcessTape(
          Input, ContextJson, CurrentState, ResolvedPersona);
      Tape.Memories.Empty();
      Tape.bVectorQueried = false;

      return detail::RunProtocolTurn(NpcId, Input, RunId, Tape, TEXT(""), false,
                                     0, Runtime, Dispatch, GetState);
    }();
  };
}

} // namespace rtk
