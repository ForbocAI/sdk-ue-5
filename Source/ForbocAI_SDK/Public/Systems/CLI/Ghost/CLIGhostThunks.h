#pragma once

#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/Ghost/GhostThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a Ghost CLI consumer, I need session creation dispatched through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &TestSuite, int32 Duration) */
template <typename RuntimeState = FRuntimeState>
inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &TestSuite,
                                    int32 Duration) {
  FGhostConfig Config;
  Config.TestSuite = TestSuite;
  Config.Duration = Duration;
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::startGhostThunk(Config)));
}

/** User Story: As a Ghost CLI consumer, I need session status selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStatus getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStatus
getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store,
               const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
}

/** User Story: As a Ghost CLI consumer, I need session results selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostResults getGhostResults(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostResults
getGhostResults(rtk::EnhancedStore<RuntimeState> &Store,
                const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostResultsThunk(SessionId)));
}

/** User Story: As a Ghost CLI consumer, I need a session stop request dispatched through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::stopGhostThunk(SessionId)));
}

/** User Story: As a Ghost CLI consumer, I need bounded session history selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FGhostHistoryEntry> getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FGhostHistoryEntry>
getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostHistoryThunk(Limit)));
}

} // namespace Ops
