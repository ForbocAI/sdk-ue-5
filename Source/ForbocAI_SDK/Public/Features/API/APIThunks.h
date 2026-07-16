#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

#include "Features/API/APIApi.h"

namespace rtk {

/**
 * Builds the thunk that checks API status for CLI doctor flows.
 * User Story: As runtime health checks, I need a lightweight doctor thunk so
 * callers can verify API availability through the store contract.
 * @fn inline ThunkAction<FApiStatusResponse, FRuntimeState> doctorThunk()
 */
inline ThunkAction<FApiStatusResponse, FRuntimeState> doctorThunk() {
  return [](std::function<AnyAction(const AnyAction &)> Dispatch,
            std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FApiStatusResponse> {
    return APISlice::Endpoints::getApiStatus()(Dispatch, GetState);
  };
}

/**
 * Builds the thunk alias for API status checks.
 * User Story: As callers using clearer naming, I need a semantic alias so
 * health checks can be invoked without duplicating implementation.
 * @fn inline ThunkAction<FApiStatusResponse, FRuntimeState> checkApiStatusThunk()
 */
inline ThunkAction<FApiStatusResponse, FRuntimeState> checkApiStatusThunk() {
  return doctorThunk();
}

} // namespace rtk
