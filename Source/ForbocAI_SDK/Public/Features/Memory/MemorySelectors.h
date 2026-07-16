#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/MemorySlice.h"

namespace MemorySelectors {

/** User Story: As a features memory consumer, I need to invoke select memory by id through a stable signature so the features memory workflow remains explicit and composable. @fn inline func::Maybe<FMemoryItem> selectMemoryById(const MemorySlice::FMemorySliceState &State, const FString &Id) */
inline func::Maybe<FMemoryItem>
selectMemoryById(const MemorySlice::FMemorySliceState &State,
                 const FString &Id) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectById(
      State.Entities, Id);
}

/** User Story: As a features memory consumer, I need to invoke select all memories through a stable signature so the features memory workflow remains explicit and composable. @fn inline TArray<FMemoryItem> selectAllMemories(const MemorySlice::FMemorySliceState &State) */
inline TArray<FMemoryItem>
selectAllMemories(const MemorySlice::FMemorySliceState &State) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectAll(
      State.Entities);
}

/** User Story: As a features memory consumer, I need to invoke select memory count through a stable signature so the features memory workflow remains explicit and composable. @fn inline int32 selectMemoryCount(const MemorySlice::FMemorySliceState &State) */
inline int32
selectMemoryCount(const MemorySlice::FMemorySliceState &State) {
  return MemorySlice::GetMemoryAdapter().getSelectors().selectTotal(
      State.Entities);
}

/** User Story: As a features memory consumer, I need to invoke select memory storage status through a stable signature so the features memory workflow remains explicit and composable. @fn inline const FString & selectMemoryStorageStatus(const MemorySlice::FMemorySliceState &State) */
inline const FString &
selectMemoryStorageStatus(const MemorySlice::FMemorySliceState &State) {
  return State.StorageStatus;
}

/** User Story: As a features memory consumer, I need to invoke select memory recall status through a stable signature so the features memory workflow remains explicit and composable. @fn inline const FString & selectMemoryRecallStatus(const MemorySlice::FMemorySliceState &State) */
inline const FString &
selectMemoryRecallStatus(const MemorySlice::FMemorySliceState &State) {
  return State.RecallStatus;
}

/** User Story: As a features memory consumer, I need to invoke select memory error through a stable signature so the features memory workflow remains explicit and composable. @fn inline const FString & selectMemoryError(const MemorySlice::FMemorySliceState &State) */
inline const FString &
selectMemoryError(const MemorySlice::FMemorySliceState &State) {
  return State.Error;
}

/** User Story: As a features memory consumer, I need to invoke select last recalled ids through a stable signature so the features memory workflow remains explicit and composable. @fn inline const TArray<FString> & selectRecalledIds(const MemorySlice::FMemorySliceState &State) */
inline const TArray<FString> &
selectRecalledIds(const MemorySlice::FMemorySliceState &State) {
  return State.RecalledIds;
}

/** User Story: As a features memory consumer, I need to invoke select last recalled memories through a stable signature so the features memory workflow remains explicit and composable. @fn inline TArray<FMemoryItem> selectRecalledMemories(const MemorySlice::FMemorySliceState &State) */
inline TArray<FMemoryItem>
selectRecalledMemories(const MemorySlice::FMemorySliceState &State) {
  const TArray<func::Maybe<FMemoryItem>> Items =
      func::map_array<FString, func::Maybe<FMemoryItem>>(
          State.RecalledIds, [&State](const FString &Id) {
            return selectMemoryById(State, Id);
          });
  return func::filter_map_array<func::Maybe<FMemoryItem>, FMemoryItem>(
      Items,
      [](const func::Maybe<FMemoryItem> &Item) { return Item.hasValue; },
      [](const func::Maybe<FMemoryItem> &Item) { return Item.value; });
}

} // namespace MemorySelectors
