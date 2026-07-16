#pragma once

#include "Core/rtk.hpp"
#include "Features/API/APIThunks.h"
#include "Features/Async/AsyncAdapters.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a features cli system consumer, I need to invoke check api status through a stable signature so the features cli system workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FApiStatusResponse checkApiStatus(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline FApiStatusResponse
checkApiStatus(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::checkApiStatusThunk()));
}

} // namespace Ops
