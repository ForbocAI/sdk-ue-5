#pragma once

#include "Core/RTK/Query/Serialization/SerializationTypes.hpp"

namespace rtk {

/** User Story: As an RTK Query serialization consumer, I need to serialize query arguments through a stable signature so cache keys remain explicit and composable. @fn inline QueryCacheKey defaultSerializeQueryArgs(const SerializeQueryArgsOptions &Options) */
inline QueryCacheKey
defaultSerializeQueryArgs(const SerializeQueryArgsOptions &Options) {
  return Options.endpointName + TEXT("(") + Options.queryArgs + TEXT(")");
}

/** User Story: As an RTK Query serialization consumer, I need to serialize typed query arguments through a stable signature so cache keys remain explicit and composable. @fn template <typename Arg> QueryCacheKey defaultSerializeQueryArgs(const FString &EndpointName, const Arg &QueryArgs) */
template <typename Arg>
QueryCacheKey defaultSerializeQueryArgs(const FString &EndpointName,
                                        const Arg &QueryArgs) {
  SerializeQueryArgsOptions Options;
  Options.endpointName = EndpointName;
  Options.queryArgs = payload_debug::DebugPayloadString(QueryArgs);
  return defaultSerializeQueryArgs(Options);
}

} // namespace rtk
