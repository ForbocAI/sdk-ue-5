#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/API/Endpoints/NPC/NPCApi.h"
#include "Systems/Actor/Local/ActorLocalThunks.h"
#include "Systems/Memory/Local/MemoryLocalThunks.h"
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
  return LocalActor::createActor(Store, Persona);
}

/** User Story: As a features cli npc consumer, I need to invoke get active npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getActiveNpc(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getActiveNpc(rtk::EnhancedStore<RuntimeState> &Store) {
  return LocalActor::getActiveActor(Store);
}

/** User Story: As a features cli npc consumer, I need to invoke get npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return LocalActor::getActor(Store, NpcId);
}

/** User Story: As normal NPC state, I need actor targeting and reduction delegated to the shared SDK actor operation. @fn template <typename RuntimeState = FRuntimeState> inline FActorUpdateResult updateNpc(rtk::EnhancedStore<RuntimeState> &Store, const FActorUpdateInput &Input) */
template <typename RuntimeState = FRuntimeState>
inline FActorUpdateResult
updateNpc(rtk::EnhancedStore<RuntimeState> &Store,
          const FActorUpdateInput &Input) {
  return LocalActor::updateActor(Store, Input);
}

/** User Story: As normal NPC processing, I need one decoded request routed only through the normal NPC API process endpoint. @fn template <typename RuntimeState = FRuntimeState> inline FAgentResponse processNpc(rtk::EnhancedStore<RuntimeState> &Store, const FProtocolProcessInput &RequestedInput) */
template <typename RuntimeState = FRuntimeState>
inline FAgentResponse processNpc(rtk::EnhancedStore<RuntimeState> &Store,
                                 const FProtocolProcessInput &RequestedInput) {
  FProtocolProcessInput Input = RequestedInput;
  const FNPCInternalState Actor = func::requireJust(
      LocalActor::ensureActor(Store, Input.NpcId),
      std::string(TCHAR_TO_UTF8(
          *ForbocAI::CLI::NPC::readCliNpcState().Messages.NotFound)));
  Input.Persona = Actor.Persona;
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::processNPC(Input,
                                     rtk::LocalProtocolHandlerContext(
                                         Input.NpcId))));
}

/** User Story: As normal NPC decision making, I need one decoded request routed through the normal NPC API endpoint with actor-scoped vector execution available for API instructions. @fn template <typename RuntimeState = FRuntimeState> inline FAgentResponse decideNpc(rtk::EnhancedStore<RuntimeState> &Store, const FProtocolProcessInput &RequestedInput) */
template <typename RuntimeState = FRuntimeState>
inline FAgentResponse decideNpc(rtk::EnhancedStore<RuntimeState> &Store,
                                const FProtocolProcessInput &RequestedInput) {
  FProtocolProcessInput Input = RequestedInput;
  const FNPCInternalState Actor = func::requireJust(
      LocalActor::ensureActor(Store, Input.NpcId),
      std::string(TCHAR_TO_UTF8(
          *ForbocAI::CLI::NPC::readCliNpcState().Messages.NotFound)));
  Input.Persona = Actor.Persona;
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::processNPC(Input,
                                     rtk::LocalProtocolHandlerContext(
                                         Input.NpcId))));
}

/** User Story: As normal NPC hydration, I need the NPC SDK feature to compose the shared local actor and vector capability. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> recallNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
recallNpc(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return LocalActor::recallActor(Store, NpcId);
}

/** User Story: As the thin UE CLI boundary, I need one SLM-generated NPC attribute returned per call, conditioned on the prior attributes supplied as context, so personas compose granularly one round trip at a time. @fn template <typename RuntimeState = FRuntimeState> inline func::AsyncResult<FNpcAttributeGenerateResponse> generateNpcAttribute(rtk::EnhancedStore<RuntimeState> &Store, const FString &Attribute, const FString &Context) */
template <typename RuntimeState = FRuntimeState>
inline func::AsyncResult<FNpcAttributeGenerateResponse>
generateNpcAttribute(rtk::EnhancedStore<RuntimeState> &Store,
                     const FString &Attribute, const FString &Context) {
  FNpcAttributeGenerateRequest Request;
  Request.Context = Context;
  return Store.dispatch(
      APISlice::Endpoints::postNpcGenerateAttribute(Attribute, Request));
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
