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

} // namespace NPCSelectors
