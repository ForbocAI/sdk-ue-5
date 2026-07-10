#pragma once

#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<TArray<FAgent>> getNPCs() {
  TArray<FApiEndpointTag> Tags;
  Tags.Add(FApiEndpointTag{TEXT("NPC"), TEXT("LIST")});
  return Detail::MakeGet<TArray<FAgent>>(
      TEXT("getNPCs"), SDKConfig::GetApiUrl() + TEXT("/npcs"), Tags);
}

inline Thunk<FAgent> getNPC(const FString &NpcId) {
  TArray<FApiEndpointTag> Tags;
  Tags.Add(FApiEndpointTag{TEXT("NPC"), NpcId});
  return Detail::MakeGet<FAgent>(TEXT("getNPC"), SDKConfig::GetApiUrl() +
                                                   TEXT("/npcs/") +
                                                   Detail::Encode(NpcId),
                                 Tags);
}

inline Thunk<FAgent> postNPC(const FAgentConfig &Config) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("NPC"), TEXT("LIST")});
  return (!Config.Id.IsEmpty()
              ? (Invalidates.Add(FApiEndpointTag{TEXT("NPC"), Config.Id}),
                 void())
              : void(),
          Detail::MakePost<FAgentConfig, FAgent>(
              TEXT("postNPC"), SDKConfig::GetApiUrl() + TEXT("/npcs"), Config,
              Invalidates));
}

inline Thunk<FNPCProcessResponse>
postNpcProcess(const FString &NpcId, const FNPCProcessRequest &Request) {
  return Detail::MakePostWithCodec<FNPCProcessRequest, FNPCProcessResponse>(
      TEXT("postNpcProcess"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/process"),
      Request, Detail::EncodeNpcProcessRequest,
      Detail::DecodeNpcProcessResponse);
}

} // namespace Endpoints
} // namespace APISlice
