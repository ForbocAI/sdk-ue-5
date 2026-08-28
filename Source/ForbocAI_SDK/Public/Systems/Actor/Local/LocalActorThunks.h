#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Components/Actor/Operations/ActorOperationTypes.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Local/LocalThunks.h"
#include "Systems/NPC/Identity/NPCIdentityAdapters.h"

struct FRuntimeState;

namespace LocalActor {

/** User Story: As either NPC or Ghost composition, I need one SDK-owned ECS actor created and persisted through its vector database. @fn template <typename RuntimeState = FRuntimeState> inline FNPCInternalState createActor(rtk::EnhancedStore<RuntimeState> &Store, const FString &Persona) */
template <typename RuntimeState = FRuntimeState>
inline FNPCInternalState createActor(rtk::EnhancedStore<RuntimeState> &Store,
                                     const FString &Persona) {
  const auto State = ForbocAI::CLI::NPC::readCliNpcState();
  FNPCInternalState Info;
  Info.Id = NPCId::GenerateNPCId();
  Info.Persona = Persona;
  Store.dispatch(NPCActions::setNPCInfo(Info));
  AsyncAdapters::waitForResult(Store.dispatch(rtk::storeNodeMemoryThunk(
      Persona, State.Syntax.PersonaMemoryType,
      MemoryConfiguration::memoryData().Defaults.Importance, Info.Id)));
  const func::Maybe<FNPCInternalState> Active =
      NPCSelectors::selectActiveNPC(Store.getState().NPCs);
  return Active.hasValue ? Active.value : Info;
}

/** User Story: As shared actor composition, I need the active ECS actor selected from the package root store. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getActiveActor(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getActiveActor(rtk::EnhancedStore<RuntimeState> &Store) {
  return NPCSelectors::selectActiveNPC(Store.getState().NPCs);
}

/** User Story: As shared actor composition, I need one explicit ECS actor selected from the package root store. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> getActor(rtk::EnhancedStore<RuntimeState> &Store, const FString &ActorId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
getActor(rtk::EnhancedStore<RuntimeState> &Store, const FString &ActorId) {
  return NPCSelectors::selectNPCById(Store.getState().NPCs, ActorId);
}

/** User Story: As shared actor state, I need an explicit or active target resolved inside the SDK before one validated delta is reduced through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FActorUpdateResult updateActor(rtk::EnhancedStore<RuntimeState> &Store, const FActorUpdateInput &Input) */
template <typename RuntimeState = FRuntimeState>
inline FActorUpdateResult
updateActor(rtk::EnhancedStore<RuntimeState> &Store,
            const FActorUpdateInput &Input) {
  const func::Maybe<FString> TargetActorId = func::match(
      Input.RequestedActorId,
      [](const FString &ActorId) { return func::just(ActorId); },
      [&Store]() {
        return func::fmap(
            getActiveActor(Store),
            [](const FNPCInternalState &Actor) { return Actor.Id; });
      });
  return func::match(
      TargetActorId,
      [&Store, &Input](const FString &ActorId) {
        Store.dispatch(NPCActions::updateNPCState(ActorId, Input.Delta));
        return FActorUpdateResult{func::just(ActorId),
                                  getActor(Store, ActorId)};
      },
      []() {
        return FActorUpdateResult{func::nothing<FString>(),
                                  func::nothing<FNPCInternalState>()};
      });
}

/** User Story: As shared actor hydration, I need canonical persona memory restored without treating ordinary observations as identity. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> recallActor(rtk::EnhancedStore<RuntimeState> &Store, const FString &ActorId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
recallActor(rtk::EnhancedStore<RuntimeState> &Store,
            const FString &ActorId) {
  const auto &Memory = MemoryConfiguration::memoryData();
  const auto ActorState = ForbocAI::CLI::NPC::readCliNpcState();
  const TArray<FMemoryItem> Records = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listNodeMemoryThunk(
          Memory.Defaults.ListLimit, Memory.Defaults.ListOffset, ActorId)));
  const func::Maybe<FMemoryItem> Persona = func::find_array<FMemoryItem>(
      Records, [&ActorState](const FMemoryItem &Item) {
        return Item.Type == ActorState.Syntax.PersonaMemoryType;
      });
  const func::Maybe<FNPCInternalState> Existing = getActor(Store, ActorId);
  return func::match(
      Persona,
      [&Store, &ActorId, &Existing](const FMemoryItem &Record) {
        FNPCInternalState Info = Existing.hasValue
                                     ? Existing.value
                                     : FNPCInternalState();
        Info.Id = ActorId;
        Info.Persona = Record.Text;
        Store.dispatch(NPCActions::setNPCInfo(Info));
        return getActor(Store, ActorId);
      },
      [&Existing]() { return Existing; });
}

/** User Story: As protocol execution, I need a missing in-memory actor rehydrated from SDK-owned vector storage. @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> ensureActor(rtk::EnhancedStore<RuntimeState> &Store, const FString &ActorId) */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState>
ensureActor(rtk::EnhancedStore<RuntimeState> &Store,
            const FString &ActorId) {
  const func::Maybe<FNPCInternalState> Existing = getActor(Store, ActorId);
  return Existing.hasValue ? Existing : recallActor(Store, ActorId);
}

} // namespace LocalActor
