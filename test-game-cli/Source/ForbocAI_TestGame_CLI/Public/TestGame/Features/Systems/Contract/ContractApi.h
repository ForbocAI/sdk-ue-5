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
    const Contract::FTestGameContractRequest Request =
        Contract::createTestGameContractRequest(ApiUrl, ApiKey);
    Options.headers = Request.Headers;
    return rtk::fetchBaseQuery<FString>(Options)(
        Request.Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{});
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
