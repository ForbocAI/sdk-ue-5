#pragma once

#include "Core/rtk.hpp"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Systems/API/APIApi.h"

namespace TestGame {
namespace ContractApi {

inline bool providesTags(
    const rtk::ApiEndpoint<FString, FString> &EndpointDefinition) {
  return EndpointDefinition.providesTags.Num() > 0;
}

inline rtk::ApiEndpoint<FString, FString> contractEndpoint() {
  rtk::ApiEndpoint<FString, FString> Endpoint;
  Endpoint.EndpointName = TEXT("getTestGameContract");
  Endpoint.providesTags = {
      rtk::FApiEndpointTag{TEXT("TestGameContract"), TEXT("CURRENT")}};
  Endpoint.RequestBuilder = [](const FString &ApiUrl) {
    rtk::FetchBaseQueryArgs Options;
    const FString ApiKey = SDKConfig::GetApiKey();
    !ApiKey.IsEmpty()
        ? (Options.headers.Add(TEXT("Authorization"),
                               FString(TEXT("Bearer ")) + ApiKey),
           void())
        : void();
    rtk::FetchArgs Args;
    const FString BaseUrl =
        ApiUrl.EndsWith(TEXT("/")) ? ApiUrl.LeftChop(1) : ApiUrl;
    Args.url = BaseUrl + TEXT("/test-game/contract");
    Args.method = TEXT("GET");
    return rtk::fetchBaseQuery<FString>(Options)(
        Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{});
  };
  check(providesTags(Endpoint));
  return Endpoint;
}

inline rtk::ThunkAction<FString, FTestGameState>
getTestGameContractThunk(const FString &ApiUrl) {
  return rtk::injectEndpoints(APISlice::api, contractEndpoint())(ApiUrl);
}

} // namespace ContractApi
} // namespace TestGame
