#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<rtk::FEmptyPayload>
postMemoryStore(const FString &NpcId,
                const FRemoteMemoryStoreRequest &Request) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Memory"), NpcId});
  return Detail::MakePost<FRemoteMemoryStoreRequest, rtk::FEmptyPayload>(
      TEXT("postMemoryStore"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/memory"),
      Request, Invalidates);
}

inline Thunk<TArray<FMemoryItem>> getMemoryList(const FString &NpcId) {
  TArray<FApiEndpointTag> Tags;
  Tags.Add(FApiEndpointTag{TEXT("Memory"), NpcId});
  return Detail::MakeGet<TArray<FMemoryItem>>(
      TEXT("getMemoryList"), SDKConfig::GetApiUrl() + TEXT("/npcs/") +
                                 Detail::Encode(NpcId) + TEXT("/memory"),
      Tags);
}

inline Thunk<TArray<FMemoryItem>>
postMemoryRecall(const FString &NpcId,
                 const FRemoteMemoryRecallRequest &Request) {
  return Detail::MakePost<FRemoteMemoryRecallRequest, TArray<FMemoryItem>>(
      TEXT("postMemoryRecall"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/memory/recall"),
      Request);
}

inline Thunk<rtk::FEmptyPayload> deleteMemoryClear(const FString &NpcId) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Memory"), NpcId});
  return Detail::MakeDelete<rtk::FEmptyPayload>(
      TEXT("deleteMemoryClear"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/memory/clear"),
      Invalidates);
}

} // namespace Endpoints
} // namespace APISlice
