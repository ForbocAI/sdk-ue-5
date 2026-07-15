#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/MemorySlice.h"

namespace MemorySelectors {

inline func::Maybe<FMemoryItem>
selectMemoryById(const MemorySlice::FMemorySliceState &State,
                 const FString &Id) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectById(
      State.Entities, Id);
}

inline TArray<FMemoryItem>
selectAllMemories(const MemorySlice::FMemorySliceState &State) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectAll(
      State.Entities);
}

inline int32
selectMemoryCount(const MemorySlice::FMemorySliceState &State) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectTotal(
      State.Entities);
}

inline const FString &
selectMemoryStorageStatus(const MemorySlice::FMemorySliceState &State) {
  return State.StorageStatus;
}

inline const FString &
selectMemoryRecallStatus(const MemorySlice::FMemorySliceState &State) {
  return State.RecallStatus;
}

inline const FString &
selectMemoryError(const MemorySlice::FMemorySliceState &State) {
  return State.Error;
}

inline const TArray<FString> &
selectLastRecalledIds(const MemorySlice::FMemorySliceState &State) {
  return State.LastRecalledIds;
}

inline void CollectLastRecalledMemories(
    const MemorySlice::FMemorySliceState &State, TArray<FMemoryItem> &Items,
    int32 Index) {
  Index >= State.LastRecalledIds.Num()
      ? void()
      : (func::match(
             selectMemoryById(State, State.LastRecalledIds[Index]),
             [&Items](const FMemoryItem &Item) {
               Items.Add(Item);
               return true;
             },
             []() { return false; }),
         CollectLastRecalledMemories(State, Items, Index + 1), void());
}

inline TArray<FMemoryItem>
selectLastRecalledMemories(const MemorySlice::FMemorySliceState &State) {
  TArray<FMemoryItem> Items;
  CollectLastRecalledMemories(State, Items, 0);
  return Items;
}

} // namespace MemorySelectors
