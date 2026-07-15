#pragma once

#include "Core/fp.hpp"
#include "Features/NPC/NPCAdapters.h"
#include "Features/NPC/NPCTypes.h"

namespace NPCSelectors {

inline func::Maybe<FNPCInternalState>
selectNPCById(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return NPCAdapters::npcAdapter().getSelectors().selectById(State.Entities,
                                                              Id);
}

inline TArray<FString>
selectNPCIds(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectIds(State.Entities);
}

inline TMap<FString, FNPCInternalState>
selectNPCEntities(const NPCSlice::FNPCSliceState &State) {
  return State.Entities.entities;
}

inline TArray<FNPCInternalState>
selectAllNPCs(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectAll(State.Entities);
}

inline int32 selectTotalNPCs(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectTotal(State.Entities);
}

inline FString selectActiveNpcId(const NPCSlice::FNPCSliceState &State) {
  return State.ActiveNpcId;
}

inline func::Maybe<FNPCInternalState>
selectActiveNPC(const NPCSlice::FNPCSliceState &State) {
  return State.ActiveNpcId.IsEmpty()
             ? func::nothing<FNPCInternalState>()
             : selectNPCById(State, State.ActiveNpcId);
}

inline func::Maybe<FAgentState>
selectNPCState(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.State; });
}

inline func::Maybe<TArray<FNPCHistoryEntry>>
selectNPCHistory(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.History; });
}

inline bool selectNPCBlocked(const NPCSlice::FNPCSliceState &State,
                             const FString &Id) {
  return func::match(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.bIsBlocked; },
      []() { return false; });
}

inline func::Maybe<FString>
selectNPCBlockReason(const NPCSlice::FNPCSliceState &State,
                     const FString &Id) {
  return func::mbind(
      selectNPCById(State, Id), [](const FNPCInternalState &Npc) {
        return Npc.BlockReason.IsEmpty() ? func::nothing<FString>()
                                         : func::just(Npc.BlockReason);
      });
}

inline func::Maybe<TArray<FNPCStateLogEntry>>
selectNPCStateLog(const NPCSlice::FNPCSliceState &State,
                  const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.StateLog; });
}

} // namespace NPCSelectors
