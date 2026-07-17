#pragma once

#include "Core/rtk.hpp"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"

namespace TestGame {
struct FTestGameState;

namespace ContractApi {

inline rtk::Api<FTestGameState> testGameApi = []() {
  const Contract::FContractApiData &Data = Contract::ContractData().Api;
  rtk::Api<FTestGameState> ApiDefinition = rtk::createApi<FTestGameState>(
      Data.ReducerPath, TArray<FString>{Data.TagType});
  check(!ApiDefinition.TagTypes.IsEmpty());
  return ApiDefinition;
}();

/** User Story: As a features systems contract consumer, I need to invoke provides tags through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline bool providesTags( const rtk::ApiEndpoint<FString, FString> &EndpointDefinition) */
inline bool providesTags(
    const rtk::ApiEndpoint<FString, FString> &EndpointDefinition) {
  return !EndpointDefinition.providesTags.IsEmpty();
}

/** User Story: As a features systems contract consumer, I need to invoke contract endpoint through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline rtk::ApiEndpoint<FString, FString> contractEndpoint() */
inline rtk::ApiEndpoint<FString, FString> contractEndpoint() {
  rtk::ApiEndpoint<FString, FString> Endpoint;
  const Contract::FContractApiData &Data = Contract::ContractData().Api;
  Endpoint.EndpointName = Data.EndpointName;
  Endpoint.providesTags = {rtk::FApiEndpointTag{Data.TagType, Data.TagId}};
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

/** User Story: As a features systems contract consumer, I need to invoke get test game contract thunk through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline rtk::ThunkAction<FString, FTestGameState> getTestGameContractThunk(const FString &ApiUrl) */
inline rtk::ThunkAction<FString, FTestGameState>
getTestGameContractThunk(const FString &ApiUrl) {
  const rtk::ApiEndpoint<FString, FString> Endpoint = contractEndpoint();
  rtk::Api<FTestGameState> &InjectedApi =
      rtk::injectEndpoints(testGameApi, Endpoint);
  return rtk::initiate(InjectedApi, Endpoint)(ApiUrl);
}

} // namespace ContractApi
} // namespace TestGame
