#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Entities/NPCs/NPCsAdapters.h"

namespace MicroGame {
namespace NPCsSelectors {

/** User Story: As a features entities npcs consumer, I need to invoke select all npcs through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline TArray<FGameNPC> SelectAllNpcs(const FNPCsSliceState &State) */
inline TArray<FGameNPC> SelectAllNpcs(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectAll(State.Entities);
}

/** User Story: As a features entities npcs consumer, I need to invoke select npc by id through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline func::Maybe<FGameNPC> SelectNpcById(const FNPCsSliceState &State, const FString &Id) */
inline func::Maybe<FGameNPC> SelectNpcById(const FNPCsSliceState &State,
                                           const FString &Id) {
  return GetNPCAdapter().getSelectors().selectById(State.Entities, Id);
}

/** User Story: As a features entities npcs consumer, I need to invoke select npc entities through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::EntityState<FGameNPC> SelectNpcEntities(const FNPCsSliceState &State) */
inline rtk::EntityState<FGameNPC>
SelectNpcEntities(const FNPCsSliceState &State) {
  return State.Entities;
}

/** User Story: As a features entities npcs consumer, I need to invoke select npc ids through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline TArray<FString> SelectNpcIds(const FNPCsSliceState &State) */
inline TArray<FString> SelectNpcIds(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectIds(State.Entities);
}

/** User Story: As a features entities npcs consumer, I need to invoke select npc total through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline int32 SelectNpcTotal(const FNPCsSliceState &State) */
inline int32 SelectNpcTotal(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectTotal(State.Entities);
}

} // namespace NPCsSelectors
} // namespace MicroGame
