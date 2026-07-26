#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As an NPC API consumer, I need the API-owned random conversation exposed through the existing RTK API root. @fn inline Thunk<FNPCConversationResponse> postNpcConversation() */
inline Thunk<FNPCConversationResponse> postNpcConversation() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakePostNoBodyWithCodec<FNPCConversationResponse>(
      Data.Names.PostNpcConversation,
      Configuration::endpointPath(
          {Data.Segments.Npcs, Data.Segments.Conversation}),
      Detail::DecodeNpcConversationResponse, TArray<FApiEndpointTag>(),
      Data.Timeouts.NpcConversationMs);
}

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
      Detail::DecodeNpcProcessResponse, Invalidates,
      Data.Timeouts.NpcProcessMs);
}

} // namespace Endpoints
} // namespace APISlice
