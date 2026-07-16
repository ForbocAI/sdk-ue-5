#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Memory/MemoryAdapters.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {
namespace GameMemorySelectors {

/** User Story: As a features systems memory consumer, I need to invoke select all memories through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline TArray<FMemoryRecord> SelectAllMemories(const FGameMemorySliceState &State) */
inline TArray<FMemoryRecord>
SelectAllMemories(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectAll(State.Entities);
}

/** User Story: As a features systems memory consumer, I need to invoke select memory by id through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline func::Maybe<FMemoryRecord> SelectMemoryById(const FGameMemorySliceState &State, const FString &Id) */
inline func::Maybe<FMemoryRecord>
SelectMemoryById(const FGameMemorySliceState &State, const FString &Id) {
  return GetGameMemoryAdapter().getSelectors().selectById(State.Entities, Id);
}

/** User Story: As a features systems memory consumer, I need to invoke select memory entities through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::EntityState<FMemoryRecord> SelectMemoryEntities(const FGameMemorySliceState &State) */
inline rtk::EntityState<FMemoryRecord>
SelectMemoryEntities(const FGameMemorySliceState &State) {
  return State.Entities;
}

/** User Story: As a features systems memory consumer, I need to invoke select memory ids through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline TArray<FString> SelectMemoryIds(const FGameMemorySliceState &State) */
inline TArray<FString> SelectMemoryIds(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectIds(State.Entities);
}

/** User Story: As a features systems memory consumer, I need to invoke select memory total through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline int32 SelectMemoryTotal(const FGameMemorySliceState &State) */
inline int32 SelectMemoryTotal(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectTotal(State.Entities);
}

/** User Story: As a features systems memory consumer, I need to invoke collect memories by npc id through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline void CollectMemoriesByNpcId( const TArray<FMemoryRecord> &Records, const FString &NpcId, int32 Index, TArray<FMemoryRecord> &Matches) */
inline void CollectMemoriesByNpcId(
    const TArray<FMemoryRecord> &Records, const FString &NpcId, int32 Index,
    TArray<FMemoryRecord> &Matches) {
  Index >= Records.Num()
      ? void()
      : (Records[Index].NpcId == NpcId
             ? (Matches.Add(Records[Index]), void())
             : void(),
         CollectMemoriesByNpcId(Records, NpcId, Index + 1, Matches));
}

/** User Story: As a features systems memory consumer, I need to invoke select memories by npc id through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline TArray<FMemoryRecord> SelectMemoriesByNpcId(const FGameMemorySliceState &State, const FString &NpcId) */
inline TArray<FMemoryRecord>
SelectMemoriesByNpcId(const FGameMemorySliceState &State,
                      const FString &NpcId) {
  const TArray<FMemoryRecord> Records = SelectAllMemories(State);
  TArray<FMemoryRecord> Matches;
  CollectMemoriesByNpcId(Records, NpcId, 0, Matches);
  return Matches;
}

} // namespace GameMemorySelectors
} // namespace TestGame
