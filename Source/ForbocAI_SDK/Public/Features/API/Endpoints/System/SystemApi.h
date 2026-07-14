#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<FApiStatusResponse> getApiStatus() {
  return Detail::MakeGet<FApiStatusResponse>(
      TEXT("getApiStatus"), SDKConfig::GetApiUrl() + TEXT("/status"));
}

} // namespace Endpoints
} // namespace APISlice
