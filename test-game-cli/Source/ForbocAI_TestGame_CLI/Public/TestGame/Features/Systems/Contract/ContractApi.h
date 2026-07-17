#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"
#include "TestGame/Features/Systems/Contract/Execution/ExecutionThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"

namespace TestGame {
namespace ContractApi {

inline rtk::Api<FTestGameState> testGameApi = []() {
  const Contract::FContractApiData &Data = Contract::ContractData().Api;
  rtk::Api<FTestGameState> ApiDefinition = rtk::createApi<FTestGameState>(
      Data.ReducerPath, TArray<FString>{Data.TagType});
  check(!ApiDefinition.TagTypes.IsEmpty());
  return ApiDefinition;
}();

/** User Story: As a features systems contract consumer, I need to invoke provides tags through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline bool providesTags( const rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState> &EndpointDefinition) */
inline bool providesTags(
    const rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState>
        &EndpointDefinition) {
  return !EndpointDefinition.providesTags.IsEmpty();
}

/** User Story: As a features systems contract consumer, I need to invoke contract endpoint through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState> contractEndpoint() */
inline rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState>
contractEndpoint() {
  rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState> Endpoint;
  const Contract::FContractApiData &Data = Contract::ContractData().Api;
  Endpoint.EndpointName = Data.EndpointName;
  Endpoint.providesTags = {rtk::FApiEndpointTag{Data.TagType, Data.TagId}};
  Endpoint.RequestBuilder = [](
                                const rtk::FEmptyPayload &,
                                const rtk::ApiContext<FTestGameState> &Context) {
    return Contract::Execution::executeContractQuery(Context);
  };
  check(providesTags(Endpoint));
  return Endpoint;
}

/** User Story: As a features systems contract consumer, I need to invoke the contract endpoint through root-store configuration so transport has one authority. @fn inline rtk::ThunkAction<FString, FTestGameState> getTestGameContractThunk() */
inline rtk::ThunkAction<FString, FTestGameState>
getTestGameContractThunk() {
  const rtk::ApiEndpoint<rtk::FEmptyPayload, FString, FTestGameState>
      Endpoint = contractEndpoint();
  rtk::Api<FTestGameState> &InjectedApi =
      rtk::injectEndpoints(testGameApi, Endpoint);
  return rtk::initiate(InjectedApi, Endpoint)(rtk::FEmptyPayload{});
}

} // namespace ContractApi
} // namespace TestGame
