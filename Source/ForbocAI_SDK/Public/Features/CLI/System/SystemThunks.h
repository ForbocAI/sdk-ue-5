#pragma once

#include "Core/rtk.hpp"
#include "Features/API/APIThunks.h"
#include "Features/Async/AsyncAdapters.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline FApiStatusResponse
checkApiStatus(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::checkApiStatusThunk()));
}

} // namespace Ops
