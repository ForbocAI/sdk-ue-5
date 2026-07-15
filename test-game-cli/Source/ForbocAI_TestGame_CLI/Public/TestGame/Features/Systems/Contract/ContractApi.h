#pragma once

#include "Core/rtk.hpp"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"

namespace TestGame {
struct FTestGameState;

namespace ContractApi {

inline rtk::Api<FTestGameState> testGameApi = []() {
  rtk::Api<FTestGameState> ApiDefinition = rtk::createApi<FTestGameState>(
      TEXT("testGameApi"), TArray<FString>{TEXT("TestGameContract")});
  check(ApiDefinition.TagTypes.Num() > 0);
  return ApiDefinition;
}();

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
    Options.headers = Contract::createTestGameAuthHeaders(ApiKey);
    rtk::FetchArgs Args;
    const FString BaseUrl = Contract::resolveTestGameApiUrl(ApiUrl);
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
  return rtk::injectEndpoints(testGameApi, contractEndpoint())(ApiUrl);
}

} // namespace ContractApi
} // namespace TestGame
