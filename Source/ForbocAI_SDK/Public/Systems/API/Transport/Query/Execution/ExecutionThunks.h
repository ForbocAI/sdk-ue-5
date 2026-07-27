#pragma once

#include "Systems/API/Transport/Query/Execution/ExecutionAdapters.h"
#include "Entities/Config/ConfigSelectors.h"
#include "Components/State/StateTypes.h"

namespace APISlice::Detail {

/**
 * User Story: As API endpoint execution, I need runtime origin and credentials selected from the current package root state before generic transport runs.
 * @fn template <typename Result> inline func::AsyncResult<rtk::QueryReturnValue<Result>> ExecuteApiBaseQuery( const FString &Method, const FString &Path, const rtk::ApiContext<FRuntimeState> &Context, const FString &Body = FString(), int32 Timeout = Transport::transportQueryData().Timeouts.Disabled)
 */
template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>> ExecuteApiBaseQuery(
    const FString &Method, const FString &Path,
    const rtk::ApiContext<FRuntimeState> &Context,
    const FString &Body = FString(),
    int32 Timeout = Transport::transportQueryData().Timeouts.Disabled) {
  const FRuntimeState &State = Context.getState();
  return ExecuteApiBaseQueryAdapter<Result>(
      Method, Path, ConfigSelectors::selectApiUrl(State),
      ConfigSelectors::selectApiKey(State), Body, Timeout);
}

} // namespace APISlice::Detail
