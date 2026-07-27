#pragma once

#include "Core/fp.hpp"
#include "Systems/NPC/NPCAdapters.h"
#include "Components/NPC/NPCTypes.h"

namespace NPCSelectors {

/** User Story: As a features npc consumer, I need to invoke select npcby id through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<FNPCInternalState> selectNPCById(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline func::Maybe<FNPCInternalState>
selectNPCById(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return NPCAdapters::npcAdapter().getSelectors().selectById(State.Entities,
                                                              Id);
}

/** User Story: As a features npc consumer, I need to invoke select npcids through a stable signature so the features npc workflow remains explicit and composable. @fn inline TArray<FString> selectNPCIds(const NPCSlice::FNPCSliceState &State) */
inline TArray<FString>
selectNPCIds(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectIds(State.Entities);
}

/** User Story: As a features npc consumer, I need to invoke select npcentities through a stable signature so the features npc workflow remains explicit and composable. @fn inline TMap<FString, FNPCInternalState> selectNPCEntities(const NPCSlice::FNPCSliceState &State) */
inline TMap<FString, FNPCInternalState>
selectNPCEntities(const NPCSlice::FNPCSliceState &State) {
  return State.Entities.entities;
}

/** User Story: As a features npc consumer, I need to invoke select all npcs through a stable signature so the features npc workflow remains explicit and composable. @fn inline TArray<FNPCInternalState> selectAllNPCs(const NPCSlice::FNPCSliceState &State) */
inline TArray<FNPCInternalState>
selectAllNPCs(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectAll(State.Entities);
}

/** User Story: As a features npc consumer, I need to invoke select total npcs through a stable signature so the features npc workflow remains explicit and composable. @fn inline int32 selectTotalNPCs(const NPCSlice::FNPCSliceState &State) */
inline int32 selectTotalNPCs(const NPCSlice::FNPCSliceState &State) {
  return NPCAdapters::npcAdapter().getSelectors().selectTotal(State.Entities);
}

/** User Story: As a features npc consumer, I need to invoke select active npc id through a stable signature so the features npc workflow remains explicit and composable. @fn inline FString selectActiveNpcId(const NPCSlice::FNPCSliceState &State) */
inline FString selectActiveNpcId(const NPCSlice::FNPCSliceState &State) {
  return State.ActiveNpcId;
}

/** User Story: As a features npc consumer, I need to invoke select active npc through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<FNPCInternalState> selectActiveNPC(const NPCSlice::FNPCSliceState &State) */
inline func::Maybe<FNPCInternalState>
selectActiveNPC(const NPCSlice::FNPCSliceState &State) {
  return State.ActiveNpcId.IsEmpty()
             ? func::nothing<FNPCInternalState>()
             : selectNPCById(State, State.ActiveNpcId);
}

/** User Story: As a features npc consumer, I need to invoke select npcstate through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<FAgentState> selectNPCState(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline func::Maybe<FAgentState>
selectNPCState(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.State; });
}

/** User Story: As a features npc consumer, I need to invoke select npchistory through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<TArray<FNPCHistoryEntry>> selectNPCHistory(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline func::Maybe<TArray<FNPCHistoryEntry>>
selectNPCHistory(const NPCSlice::FNPCSliceState &State, const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.History; });
}

/** User Story: As a features npc consumer, I need to invoke select npcblocked through a stable signature so the features npc workflow remains explicit and composable. @fn inline bool selectNPCBlocked(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline bool selectNPCBlocked(const NPCSlice::FNPCSliceState &State,
                             const FString &Id) {
  return func::match(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.bIsBlocked; },
      []() { return false; });
}

/** User Story: As a features npc consumer, I need to invoke select npcblock reason through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<FString> selectNPCBlockReason(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline func::Maybe<FString>
selectNPCBlockReason(const NPCSlice::FNPCSliceState &State,
                     const FString &Id) {
  return func::mbind(
      selectNPCById(State, Id), [](const FNPCInternalState &Npc) {
        return Npc.BlockReason.IsEmpty() ? func::nothing<FString>()
                                         : func::just(Npc.BlockReason);
      });
}

/** User Story: As a features npc consumer, I need to invoke select npcstate log through a stable signature so the features npc workflow remains explicit and composable. @fn inline func::Maybe<TArray<FNPCStateLogEntry>> selectNPCStateLog(const NPCSlice::FNPCSliceState &State, const FString &Id) */
inline func::Maybe<TArray<FNPCStateLogEntry>>
selectNPCStateLog(const NPCSlice::FNPCSliceState &State,
                  const FString &Id) {
  return func::fmap(
      selectNPCById(State, Id),
      [](const FNPCInternalState &Npc) { return Npc.StateLog; });
}

} // namespace NPCSelectors
