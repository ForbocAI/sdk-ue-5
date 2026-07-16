#pragma once

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "Features/Memory/MemoryThunks.h"

struct FRuntimeState;

namespace Ops {

/**
 * User Story: As a CLI memory consumer, I need persisted NPC records listed through the SDK so the CLI remains a thin wrapper.
 * @fn template <typename RuntimeState = FRuntimeState> inline TArray<FMemoryItem> listMemory( rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId)
 */
template <typename RuntimeState = FRuntimeState>
inline TArray<FMemoryItem> listMemory(
    rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  const auto &Data = MemoryConfiguration::memoryData();
  return AsyncAdapters::waitForResult(Store.dispatch(rtk::listNodeMemoryThunk(
      Data.Defaults.ListLimit, Data.Defaults.ListOffset, NpcId)));
}

/** User Story: As a CLI memory consumer, I need semantic recall delegated to the NPC-scoped SDK database so the CLI owns no vector behavior. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FMemoryItem> recallMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const FString &Query, int32 Limit, float Threshold) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FMemoryItem>
recallMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId,
             const FString &Query, int32 Limit, float Threshold) {
  return AsyncAdapters::waitForResult(Store.dispatch(
      rtk::recallNodeMemoryThunk(Query, Limit, Threshold, NpcId)));
}

/** User Story: As a CLI memory consumer, I need observations persisted through the SDK so the CLI owns no storage behavior. @fn template <typename RuntimeState = FRuntimeState> inline FMemoryItem storeMemory(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId, const FString &Observation, float Importance) */
template <typename RuntimeState = FRuntimeState>
inline FMemoryItem storeMemory(rtk::EnhancedStore<RuntimeState> &Store,
                               const FString &NpcId,
                               const FString &Observation,
                               float Importance) {
  return AsyncAdapters::waitForResult(Store.dispatch(rtk::storeNodeMemoryThunk(
      Observation, MemoryConfiguration::memoryData().Defaults.MemoryType,
      Importance, NpcId)));
}

/**
 * User Story: As a CLI memory consumer, I need NPC storage cleared through the SDK so Redux only resets after persistence succeeds.
 * @fn template <typename RuntimeState = FRuntimeState> inline rtk::FEmptyPayload clearMemory( rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId)
 */
template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload clearMemory(
    rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::clearNodeMemoryThunk(NpcId)));
}

} // namespace Ops
