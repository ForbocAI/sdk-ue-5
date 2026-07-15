#pragma once

// The CLI/Ghost folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Ghost/GhostThunks.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &TestSuite,
                                    int32 Duration = 300) {
  FGhostConfig Config;
  Config.TestSuite = TestSuite;
  Config.Duration = Duration;
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::startGhostThunk(Config)));
}

template <typename RuntimeState = FRuntimeState>
inline FGhostStatusResponse
getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store,
               const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
}

template <typename RuntimeState = FRuntimeState>
inline FGhostResultsResponse
getGhostResults(rtk::EnhancedStore<RuntimeState> &Store,
                const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostResultsThunk(SessionId)));
}

template <typename RuntimeState = FRuntimeState>
inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::stopGhostThunk(SessionId)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FGhostHistoryEntry>
getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = 10) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostHistoryThunk(Limit)));
}

} // namespace Ops
