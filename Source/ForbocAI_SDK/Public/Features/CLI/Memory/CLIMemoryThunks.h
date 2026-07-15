#pragma once

// The CLI/Memory folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Memory/MemoryThunks.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline TArray<FMemoryItem> listMemory(rtk::EnhancedStore<RuntimeState> &Store,
                                      const FString &NpcId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listMemoryRemoteThunk(NpcId)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FMemoryItem>
recallMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId,
             const FString &Query, int32 Limit = 10,
             float Threshold = 0.7f) {
  TArray<FMemoryItem> Results = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::recallMemoryRemoteThunk(NpcId, Query, Threshold)));
  (Limit >= 0 && Results.Num() > Limit) ? (Results.SetNum(Limit), void())
                                        : void();
  return Results;
}

template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload
storeMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId,
            const FString &Observation, float Importance = 0.8f) {
  return AsyncAdapters::waitForResult(Store.dispatch(
      rtk::storeMemoryRemoteThunk(NpcId, Observation, Importance)));
}

template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload
clearMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::clearMemoryRemoteThunk(NpcId)));
}

template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload
initNodeMemory(rtk::EnhancedStore<RuntimeState> &Store,
               const FString &DatabasePath = TEXT("")) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::initNodeMemoryThunk(DatabasePath)));
}

template <typename RuntimeState = FRuntimeState>
inline FMemoryItem storeNodeMemory(rtk::EnhancedStore<RuntimeState> &Store,
                                   const FString &Text,
                                   float Importance = 0.8f) {
  return AsyncAdapters::waitForResult(Store.dispatch(
      rtk::storeNodeMemoryThunk(Text, TEXT("observation"), Importance)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FMemoryItem>
recallNodeMemory(rtk::EnhancedStore<RuntimeState> &Store,
                 const FString &Query, int32 Limit = 10,
                 float Threshold = 0.7f) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::recallNodeMemoryThunk(Query, Limit, Threshold)));
}

template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload
clearNodeMemory(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::clearNodeMemoryThunk()));
}

} // namespace Ops
