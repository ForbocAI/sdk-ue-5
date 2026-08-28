#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Memory/MemoryAdapters.h"
#include "MicroGame/Features/Systems/Memory/SystemsMemoryTypes.h"

namespace MicroGame {
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

/** User Story: As a features systems memory consumer, I need to invoke select memories by npc id through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline TArray<FMemoryRecord> SelectMemoriesByNpcId(const FGameMemorySliceState &State, const FString &NpcId) */
inline TArray<FMemoryRecord>
SelectMemoriesByNpcId(const FGameMemorySliceState &State,
                      const FString &NpcId) {
  return func::filter_array<FMemoryRecord>(
      SelectAllMemories(State), [&NpcId](const FMemoryRecord &Record) {
        return Record.NpcId == NpcId;
      });
}

} // namespace GameMemorySelectors
} // namespace MicroGame
