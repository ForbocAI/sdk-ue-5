#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/Ghost/GhostAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<FGhostRunResponse> postGhostRun(const FGhostRunRequest &Request) {
  return Detail::MakePostWithCodec<FGhostRunRequest, FGhostRunResponse>(
      TEXT("postGhostRun"), SDKConfig::GetApiUrl() + TEXT("/ghost/run"),
      Request, Detail::ToJson<FGhostRunRequest>,
      Detail::DecodeGhostRunResponse);
}

inline Thunk<FGhostRunResponse> postGhostRun(const FGhostConfig &Config) {
  return postGhostRun(
      TypeFactory::GhostRunRequest(Config.TestSuite, Config.Duration));
}

inline Thunk<FGhostStatusResponse> getGhostStatus(const FString &SessionId) {
  return Detail::MakeGetWithCodec<FGhostStatusResponse>(
      TEXT("getGhostStatus"),
      SDKConfig::GetApiUrl() + TEXT("/ghost/") + Detail::Encode(SessionId) +
          TEXT("/status"),
      Detail::DecodeGhostStatusResponse);
}

inline Thunk<FGhostResultsResponse> getGhostResults(const FString &SessionId) {
  return Detail::MakeGetWithCodec<FGhostResultsResponse>(
      TEXT("getGhostResults"),
      SDKConfig::GetApiUrl() + TEXT("/ghost/") + Detail::Encode(SessionId) +
          TEXT("/results"),
      Detail::DecodeGhostResultsResponse);
}

inline Thunk<FGhostStopResponse> postGhostStop(const FString &SessionId) {
  return Detail::MakePostRawWithCodec<FGhostStopResponse>(
      TEXT("postGhostStop"),
      SDKConfig::GetApiUrl() + TEXT("/ghost/") + Detail::Encode(SessionId) +
          TEXT("/stop"),
      TEXT("{}"), Detail::DecodeGhostStopResponse);
}

inline Thunk<FGhostHistoryResponse> getGhostHistory(int32 Limit = 10) {
  return Detail::MakeGetWithCodec<FGhostHistoryResponse>(
      TEXT("getGhostHistory"),
      SDKConfig::GetApiUrl() + TEXT("/ghost/history?limit=") +
          FString::FromInt(Limit),
      Detail::DecodeGhostHistoryResponse);
}

} // namespace Endpoints
} // namespace APISlice
