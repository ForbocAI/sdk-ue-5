#pragma once

#include "Systems/API/APIAdapters.h"
#include "Components/API/Endpoints/EndpointsTypes.h"
#include "Components/NPC/Generate/GenerateTypes.h"
#include "Systems/API/Endpoints/NPC/Generate/GenerateAdapters.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As an NPC API consumer, I need one attribute generated per round trip, conditioned on the prior attributes I supply as context, so personas compose granularly one round trip at a time. @fn inline Thunk<FNpcAttributeGenerateResponse> postNpcGenerateAttribute(const FString &Attribute, const FNpcAttributeGenerateRequest &Request) */
inline Thunk<FNpcAttributeGenerateResponse>
postNpcGenerateAttribute(const FString &Attribute,
                         const FNpcAttributeGenerateRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakePostWithCodec<FNpcAttributeGenerateRequest,
                                   FNpcAttributeGenerateResponse>(
      Data.Names.PostNpcGenerateAttribute,
      Configuration::endpointPath(
          {Data.Segments.Npcs, Data.Segments.Generate, Attribute}),
      Request, Detail::EncodeNpcAttributeGenerateRequest,
      Detail::DecodeNpcAttributeGenerateResponse,
      TArray<rtk::FApiEndpointTag>(), Data.Timeouts.NpcGenerateMs);
}

/** User Story: As a api endpoints npc consumer, I need to invoke post npc process through a stable signature so the api endpoints npc workflow remains explicit and composable. @fn inline Thunk<FNPCProcessResponse> postNpcProcess(const FString &NpcId, const FNPCProcessRequest &Request) */
inline Thunk<FNPCProcessResponse>
postNpcProcess(const FString &NpcId, const FNPCProcessRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Invalidates{npcTagAdapter(NpcId)};
  return Detail::MakePostWithCodec<FNPCProcessRequest, FNPCProcessResponse>(
      Data.Names.PostNpcProcess,
      Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Process}),
      Request, Detail::EncodeNpcProcessRequest,
      Detail::DecodeNpcProcessResponse, Invalidates,
      Data.Timeouts.NpcProcessMs);
}

} // namespace Endpoints
} // namespace APISlice
