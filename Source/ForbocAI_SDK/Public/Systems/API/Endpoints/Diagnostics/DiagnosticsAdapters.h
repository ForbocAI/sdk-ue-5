#pragma once

#include "Systems/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Systems/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the diagnostics cache owner, I need health state isolated from NPC state so status requests cannot invalidate NPC entities. @fn inline rtk::FApiEndpointTag diagnosticsTagAdapter() */
inline rtk::FApiEndpointTag diagnosticsTagAdapter() {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.System);
}

} // namespace APISlice::Endpoints
