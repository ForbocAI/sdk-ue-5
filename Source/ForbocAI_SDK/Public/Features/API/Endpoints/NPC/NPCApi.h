#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints npc consumer, I need to invoke post npc process through a stable signature so the api endpoints npc workflow remains explicit and composable. @fn inline Thunk<FNPCProcessResponse> postNpcProcess(const FString &NpcId, const FNPCProcessRequest &Request) */
inline Thunk<FNPCProcessResponse>
postNpcProcess(const FString &NpcId, const FNPCProcessRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Invalidates{npcTagAdapter(NpcId)};
  return Detail::MakePostWithCodec<FNPCProcessRequest, FNPCProcessResponse>(
      Data.Names.PostNpcProcess,
      Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Process}),
      Request, Detail::EncodeNpcProcessRequest,
      Detail::DecodeNpcProcessResponse, Invalidates);
}

} // namespace Endpoints
} // namespace APISlice
