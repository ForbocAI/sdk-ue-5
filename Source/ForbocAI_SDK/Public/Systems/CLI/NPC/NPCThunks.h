#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/API/Endpoints/NPC/NPCApi.h"
#include "Systems/Memory/Local/LocalThunks.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Systems/Protocol/ProtocolThunks.h"
#include "Systems/Soul/SoulThunks.h"
#include "Systems/Soul/SoulAdapters.h"
#include "Systems/NPC/Identity/NPCIdentityAdapters.h"

struct FRuntimeState;

namespace Ops {

/**
 * User Story: As a CLI Soul importer, I need every authenticated memory restored through the SDK-owned NPC database before import completes.
 * @fn template <typename RuntimeState> inline void importNpcSoulMemories(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const TArray<FMemoryItem> &Memories, int32 Index = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA)
 */
template <typename RuntimeState>
inline void importNpcSoulMemories(rtk::EnhancedStore<RuntimeState> &Store,
                                  const FString &NpcId,
                                  const TArray<FMemoryItem> &Memories,
                                  int32 Index = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {
  Index >= Memories.Num()
      ? void()
      : (static_cast<void>(AsyncAdapters::waitForResult(Store.dispatch(
             rtk::upsertNodeMemoryThunk(Memories[Index], NpcId)))),
         importNpcSoulMemories(Store, NpcId, Memories, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4));
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
      Store.dispatch(rtk::processNPC(NpcId, Text, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF54CAD9838EB), TEXT(""),
                                     FAgentState(),
                                     rtk::LocalProtocolHandlerContext(NpcId))));
}

/** User Story: As the thin UE CLI boundary, I need the API-owned random NPC conversation dispatched through the package root store while remaining lazy until the command edge awaits it. @fn template <typename RuntimeState = FRuntimeState> inline func::AsyncResult<FNPCConversationResponse> converseNpcs(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline func::AsyncResult<FNPCConversationResponse>
converseNpcs(rtk::EnhancedStore<RuntimeState> &Store) {
  return Store.dispatch(APISlice::Endpoints::postNpcConversation());
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
