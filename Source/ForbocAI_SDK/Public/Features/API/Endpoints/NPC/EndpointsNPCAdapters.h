#pragma once

#include "Features/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Features/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the NPC cache owner, I need process state identified by NPC so mutations invalidate only the affected entity. @fn inline FApiEndpointTag npcTagAdapter(const FString &NpcId) */
inline FApiEndpointTag npcTagAdapter(const FString &NpcId) {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.Npc, NpcId);
}

} // namespace APISlice::Endpoints
