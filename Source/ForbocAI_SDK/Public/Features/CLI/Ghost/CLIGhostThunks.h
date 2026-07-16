#pragma once

// The CLI/Ghost folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Ghost/GhostThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a features cli ghost consumer, I need to invoke start ghost through a stable signature so the features cli ghost workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &TestSuite, int32 Duration = 300) */
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

/** User Story: As a features cli ghost consumer, I need to invoke get ghost status through a stable signature so the features cli ghost workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStatus getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStatus
getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store,
               const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
}

/** User Story: As a features cli ghost consumer, I need to invoke get ghost results through a stable signature so the features cli ghost workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FGhostResults getGhostResults(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostResults
getGhostResults(rtk::EnhancedStore<RuntimeState> &Store,
                const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostResultsThunk(SessionId)));
}

/** User Story: As a features cli ghost consumer, I need to invoke stop ghost through a stable signature so the features cli ghost workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::stopGhostThunk(SessionId)));
}

/** User Story: As a features cli ghost consumer, I need to invoke get ghost history through a stable signature so the features cli ghost workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FGhostHistoryEntry> getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = 10) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FGhostHistoryEntry>
getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = 10) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostHistoryThunk(Limit)));
}

} // namespace Ops
