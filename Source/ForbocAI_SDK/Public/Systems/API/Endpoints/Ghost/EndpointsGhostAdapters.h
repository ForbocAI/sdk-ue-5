#pragma once

#include "Systems/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Systems/API/Endpoints/Ghost/Lifecycle/EndpointsGhostLifecycleAdapters.h"
#include "Systems/API/Endpoints/Ghost/Results/EndpointsGhostResultsAdapters.h"
#include "Systems/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the Ghost cache owner, I need each run identified by session so polling and stop operations share one entity cache. @fn inline rtk::FApiEndpointTag ghostTagAdapter(const FString &SessionId) */
inline rtk::FApiEndpointTag ghostTagAdapter(const FString &SessionId) {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.Ghost, SessionId);
}

/** User Story: As the Ghost cache owner, I need one authored list identity for run history. @fn inline rtk::FApiEndpointTag ghostListTagAdapter() */
inline rtk::FApiEndpointTag ghostListTagAdapter() {
  return Configuration::endpointListTag(
      Transport::transportQueryData().Tags.Ghost);
}

} // namespace APISlice::Endpoints
