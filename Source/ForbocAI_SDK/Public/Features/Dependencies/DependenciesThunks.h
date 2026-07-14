#pragma once

#include "Core/rtk.hpp"
#include "Features/Dependencies/DependenciesAdapters.h"
#include "Features/Dependencies/DependenciesSelectors.h"

struct FRuntimeState;

namespace rtk {

inline const AsyncThunkConfig<FNativeDependenciesReport, FEmptyPayload, FRuntimeState> &
checkNativeDependenciesThunk() {
  const ConditionCallback<FEmptyPayload, FRuntimeState> Condition =
      [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FNativeDependenciesReport, FEmptyPayload, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FNativeDependenciesReport, FEmptyPayload, FRuntimeState>(
          TEXT("node/dependencies/check"),
          [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FNativeDependenciesReport> {
            return detail::ResolveAsync(
                DependenciesAdapters::checkNativeDependenciesAdapter());
          }, Condition);
  return Thunk;
}

inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> &
setupNativeDependenciesThunk() {
  const ConditionCallback<FDependenciesOptions, FRuntimeState> Condition =
      [](const FDependenciesOptions &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FDependenciesResult, FDependenciesOptions, FRuntimeState>(
          TEXT("node/dependencies/run"),
          [](const FDependenciesOptions &Options, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FDependenciesResult> {
            return detail::ResolveAsync(
                DependenciesAdapters::setupNativeDependenciesAdapter(Options));
          }, Condition);
  return Thunk;
}

inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> &
refreshNativeDependenciesThunk() {
  const ConditionCallback<FDependenciesOptions, FRuntimeState> Condition =
      [](const FDependenciesOptions &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FDependenciesResult, FDependenciesOptions, FRuntimeState>(
          TEXT("node/dependencies/refresh"),
          [](const FDependenciesOptions &Options, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FDependenciesResult> {
            FDependenciesOptions RefreshOptions = Options;
            RefreshOptions.bForce = true;
            return detail::ResolveAsync(
                DependenciesAdapters::setupNativeDependenciesAdapter(RefreshOptions));
          }, Condition);
  return Thunk;
}

} // namespace rtk
