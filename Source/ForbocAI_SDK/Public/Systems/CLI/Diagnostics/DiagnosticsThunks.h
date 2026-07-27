#pragma once

#include "Core/rtk.hpp"
#include "Systems/API/APIThunks.h"
#include "Systems/Async/AsyncAdapters.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a CLI diagnostics consumer, I need API status delegated through the SDK root store. @fn template <typename RuntimeState = FRuntimeState> inline FApiStatusResponse checkApiStatus(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline FApiStatusResponse
checkApiStatus(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::checkApiStatusThunk()));
}

/** User Story: As an SDK CLI caller, I need test-contract transport dispatched through the package root store while remaining lazy until the command edge awaits it. @fn template <typename RuntimeState = FRuntimeState> inline func::AsyncResult<FString> getMicroGameContract(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline func::AsyncResult<FString>
getMicroGameContract(rtk::EnhancedStore<RuntimeState> &Store) {
  return Store.dispatch(rtk::getMicroGameContractThunk());
}

} // namespace Ops
