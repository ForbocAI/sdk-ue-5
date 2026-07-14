#pragma once

#include "Features/Async/AsyncAdapters.h"
#include "Features/Dependencies/DependenciesThunks.h"

struct FRuntimeState;

namespace Ops {

inline FNativeDependenciesReport
checkNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::checkNativeDependenciesThunk()(rtk::FEmptyPayload{})));
}

inline FDependenciesResult
setupNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FDependenciesOptions &Options = FDependenciesOptions()) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::setupNativeDependenciesThunk()(Options)), 180.0);
}

inline FDependenciesResult
refreshNativeDependencies(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FDependenciesOptions &Options = FDependenciesOptions()) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::refreshNativeDependenciesThunk()(Options)), 180.0);
}

} // namespace Ops
