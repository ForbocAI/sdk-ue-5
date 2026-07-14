#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Memory/MemoryAdapters.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {
namespace GameMemorySelectors {

inline TArray<FMemoryRecord>
SelectAllMemories(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectAll(State.Entities);
}

inline func::Maybe<FMemoryRecord>
SelectMemoryById(const FGameMemorySliceState &State, const FString &Id) {
  return GetGameMemoryAdapter().getSelectors().selectById(State.Entities, Id);
}

inline rtk::EntityState<FMemoryRecord>
SelectMemoryEntities(const FGameMemorySliceState &State) {
  return State.Entities;
}

inline TArray<FString> SelectMemoryIds(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectIds(State.Entities);
}

inline int32 SelectMemoryTotal(const FGameMemorySliceState &State) {
  return GetGameMemoryAdapter().getSelectors().selectTotal(State.Entities);
}

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
