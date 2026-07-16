#pragma once

#include "Features/Async/AsyncAdapters.h"
#include "Features/Dependencies/DependenciesThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a features cli native consumer, I need to invoke check native dependencies through a stable signature so the features cli native workflow remains explicit and composable. @fn inline FNativeDependenciesReport checkNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store) */
inline FNativeDependenciesReport
checkNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::checkNativeDependenciesThunk()(rtk::FEmptyPayload{})));
}

/** User Story: As a features cli native consumer, I need to invoke setup native dependencies through a stable signature so the features cli native workflow remains explicit and composable. @fn inline FDependenciesResult setupNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store, const FDependenciesOptions &Options = FDependenciesOptions()) */
inline FDependenciesResult
setupNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FDependenciesOptions &Options = FDependenciesOptions()) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::setupNativeDependenciesThunk()(Options)), 180.0);
}

/** User Story: As a features cli native consumer, I need to invoke refresh native dependencies through a stable signature so the features cli native workflow remains explicit and composable. @fn inline FDependenciesResult refreshNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store, const FDependenciesOptions &Options = FDependenciesOptions()) */
inline FDependenciesResult
refreshNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FDependenciesOptions &Options = FDependenciesOptions()) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::refreshNativeDependenciesThunk()(Options)), 180.0);
}

} // namespace Ops
