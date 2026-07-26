#pragma once

#include "Features/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Features/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the System cache owner, I need health state isolated from NPC state so status requests cannot invalidate NPC entities. @fn inline rtk::FApiEndpointTag systemTagAdapter() */
inline rtk::FApiEndpointTag systemTagAdapter() {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.System);
}

} // namespace APISlice::Endpoints
