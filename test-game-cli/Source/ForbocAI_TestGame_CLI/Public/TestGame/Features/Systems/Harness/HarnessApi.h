#pragma once
/**
 * Test-game Harness RTK Query API.
 *
 * Server-data request construction and endpoint metadata live here so Harness
 * thunks can initiate API work without hand-building request/response flows.
 */

#include "Core/rtk.hpp"
#include "TestGame/TestGameStore.h"

namespace TestGame {
namespace HarnessApi {

inline rtk::Api<FTestGameState> testGameApi() {
  static const rtk::Api<FTestGameState> Api =
      rtk::createApi<FTestGameState>(TEXT("testGameApi"),
                                     TArray<FString>{TEXT("RuntimeStatus")});
  return Api;
}

inline bool providesTags(
    const rtk::ApiEndpoint<FString, FString> &Endpoint) {
  return Endpoint.ProvidesTags.Num() > 0;
}

inline rtk::ApiEndpoint<FString, FString> runtimeStatusEndpoint() {
  rtk::ApiEndpoint<FString, FString> Endpoint;
  Endpoint.EndpointName = TEXT("getRuntimeStatus");
  Endpoint.ProvidesTags = {
      rtk::FApiEndpointTag{TEXT("RuntimeStatus"), TEXT("LIST")}};
  Endpoint.RequestBuilder = [](const FString &Url) {
    rtk::FetchArgs Args;
    Args.url = Url;
    Args.method = TEXT("GET");
    return rtk::fetchBaseQuery<FString>(rtk::FetchBaseQueryArgs())(
        Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{});
  };
  return Endpoint;
}

inline rtk::ThunkAction<FString, FTestGameState>
getRuntimeStatusThunk(const FString &Url) {
  const rtk::ApiEndpoint<FString, FString> Endpoint =
      runtimeStatusEndpoint();
  check(providesTags(Endpoint));
  return rtk::injectEndpoints(testGameApi(), Endpoint)(Url);
}

} // namespace HarnessApi
} // namespace TestGame
