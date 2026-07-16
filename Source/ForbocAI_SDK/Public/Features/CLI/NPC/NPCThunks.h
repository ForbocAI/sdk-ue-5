#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Memory/Local/MemoryLocalThunks.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Soul/SoulThunks.h"
#include "Features/Soul/SoulAdapters.h"
#include "NPC/NPCId.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a CLI Soul importer, I need every authenticated memory restored through the SDK-owned NPC database before import completes. @fn template <typename RuntimeState> inline void importNpcSoulMemories(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const TArray<FMemoryItem> &Memories, int32 Index = 0) */
template <typename RuntimeState>
inline void importNpcSoulMemories(rtk::EnhancedStore<RuntimeState> &Store,
                                  const FString &NpcId,
                                  const TArray<FMemoryItem> &Memories,
                                  int32 Index = 0) {
  Index >= Memories.Num()
      ? void()
      : (static_cast<void>(AsyncAdapters::waitForResult(Store.dispatch(
             rtk::upsertNodeMemoryThunk(Memories[Index], NpcId)))),
         importNpcSoulMemories(Store, NpcId, Memories, Index + 1));
}

/** User Story: As a features cli npc consumer, I need to invoke create npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FNPCInternalState createNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &Persona) */
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

/** User Story: As a features cli npc consumer, I need to invoke get active npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getActiveNpc(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getActiveNpc(rtk::EnhancedStore<RuntimeState> &Store) {
  return NPCSelectors::selectActiveNPC(Store.getState().NPCs);
}

/** User Story: As a features cli npc consumer, I need to invoke get npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return NPCSelectors::selectNPCById(Store.getState().NPCs, NpcId);
}

/** User Story: As a features cli npc consumer, I need to invoke update npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> updateNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const FAgentState &Delta) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
updateNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId,
          const FAgentState &Delta) {
  Store.dispatch(NPCActions::updateNPCState(NpcId, Delta));
  return NPCSelectors::selectNPCById(Store.getState().NPCs, NpcId);
}

/** User Story: As a features cli npc consumer, I need to invoke process npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FAgentResponse processNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const FString &Text) */
template <typename RuntimeState = FRuntimeState>
inline FAgentResponse processNpc(rtk::EnhancedStore<RuntimeState> &Store,
                                 const FString &NpcId, const FString &Text) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::processNPC(NpcId, Text, TEXT("{}"), TEXT(""),
                                     FAgentState(),
                                     rtk::LocalProtocolHandlerContext(NpcId))));
}

/** User Story: As a features cli npc consumer, I need to invoke import npc from soul through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FImportedNpc importNpcFromSoul(rtk::EnhancedStore<RuntimeState> &Store, const FString &TxId) */
template <typename RuntimeState = FRuntimeState>
inline FImportedNpc
importNpcFromSoul(rtk::EnhancedStore<RuntimeState> &Store,
                  const FString &TxId) {
  const FSoul Soul = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::importSoulThunk()(TxId)));
  FNPCInternalState Npc;
  Npc.Id = Soul.Id;
  Npc.Persona = Soul.StructuredPersona;
  Npc.State = Soul.State;
  Store.dispatch(NPCActions::setNPCInfo(Npc));
  importNpcSoulMemories(Store, Soul.Id, Soul.Memories);
  return SoulAdapters::toImportedNpcAdapter(Soul);
}

} // namespace Ops
