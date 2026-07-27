#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Systems/Dependencies/DependenciesAdapters.h"
#include "Entities/Dependencies/DependenciesSelectors.h"

struct FRuntimeState;

namespace rtk {

/** User Story: As a features dependencies consumer, I need to invoke check native dependencies thunk through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline const AsyncThunkConfig<FNativeDependenciesReport, FEmptyPayload, FRuntimeState> & checkNativeDependenciesThunk() */
inline const AsyncThunkConfig<FNativeDependenciesReport, FEmptyPayload, FRuntimeState> &
checkNativeDependenciesThunk() {
  const ConditionCallback<FEmptyPayload, FRuntimeState> Condition =
      [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FNativeDependenciesReport, FEmptyPayload, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FNativeDependenciesReport, FEmptyPayload, FRuntimeState>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV38BD891B7072),
          [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FNativeDependenciesReport> {
            return detail::ResolveAsync(
                DependenciesAdapters::checkNativeDependenciesAdapter());
          }, Condition);
  return Thunk;
}

/** User Story: As a features dependencies consumer, I need to invoke setup native dependencies thunk through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> & setupNativeDependenciesThunk() */
inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> &
setupNativeDependenciesThunk() {
  const ConditionCallback<FDependenciesOptions, FRuntimeState> Condition =
      [](const FDependenciesOptions &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FDependenciesResult, FDependenciesOptions, FRuntimeState>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCB704413F71A),
          [](const FDependenciesOptions &Options, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FDependenciesResult> {
            return detail::ResolveAsync(
                DependenciesAdapters::setupNativeDependenciesAdapter(Options));
          }, Condition);
  return Thunk;
}

/** User Story: As a features dependencies consumer, I need to invoke refresh native dependencies thunk through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> & refreshNativeDependenciesThunk() */
inline const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState> &
refreshNativeDependenciesThunk() {
  const ConditionCallback<FDependenciesOptions, FRuntimeState> Condition =
      [](const FDependenciesOptions &, const ThunkApi<FRuntimeState> &Api) {
        return !DependenciesSelectors::selectDependenciesBusy(Api.getState());
      };
  static const AsyncThunkConfig<FDependenciesResult, FDependenciesOptions, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FDependenciesResult, FDependenciesOptions, FRuntimeState>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB48B975A015C),
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
