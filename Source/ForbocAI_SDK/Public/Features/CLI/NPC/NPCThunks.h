#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Soul/SoulThunks.h"
#include "NPC/NPCId.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline FNPCInternalState createNpc(rtk::EnhancedStore<RuntimeState> &Store,
                                   const FString &Persona) {
  FNPCInternalState Info;
  Info.Id = NPCId::GenerateNPCId();
  Info.Persona = Persona;
  Store.dispatch(NPCActions::setNPCInfo(Info));
  const func::Maybe<FNPCInternalState> Active =
      NPCSelectors::selectActiveNPC(Store.getState().NPCs);
  return Active.hasValue ? Active.value : Info;
}

template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getActiveNpc(rtk::EnhancedStore<RuntimeState> &Store) {
  return NPCSelectors::selectActiveNPC(Store.getState().NPCs);
}

template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return NPCSelectors::selectNPCById(Store.getState().NPCs, NpcId);
}

template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
updateNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId,
          const FAgentState &Delta) {
  Store.dispatch(NPCActions::updateNPCState(NpcId, Delta));
  return NPCSelectors::selectNPCById(Store.getState().NPCs, NpcId);
}

template <typename RuntimeState = FRuntimeState>
inline FAgentResponse processNpc(rtk::EnhancedStore<RuntimeState> &Store,
                                 const FString &NpcId, const FString &Text) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::processNPC(NpcId, Text, TEXT("{}"), TEXT(""),
                                     FAgentState(),
                                     rtk::InMemoryProtocolHandlerContext())));
}

template <typename RuntimeState = FRuntimeState>
inline FImportedNpc
importNpcFromSoul(rtk::EnhancedStore<RuntimeState> &Store,
                  const FString &TxId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::importNpcFromSoulThunk(TxId)));
}

} // namespace Ops
