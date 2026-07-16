#pragma once

#include "Core/RTK/Query/Types/QueryTypes.hpp"

namespace rtk {

struct SerializeQueryArgsOptions {
  FString endpointName;
  FString queryArgs;
};

typedef std::function<QueryCacheKey(const SerializeQueryArgsOptions &)>
    SerializeQueryArgs;

} // namespace rtk
