#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/Ghost/GhostAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints ghost consumer, I need to invoke post ghost run through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostRunResponse> postGhostRun(const FGhostRunRequest &Request) */
inline Thunk<FGhostRunResponse> postGhostRun(const FGhostRunRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Invalidates{ghostListTagAdapter()};
  return Detail::MakePostWithCodec<FGhostRunRequest, FGhostRunResponse>(
      Data.Names.PostGhostRun,
      Configuration::endpointPath(
          {Data.Segments.Ghost, Data.Segments.Run}),
      Request, Detail::ToJson<FGhostRunRequest>,
      Detail::DecodeGhostRunResponse, Invalidates);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke post ghost run through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostRunResponse> postGhostRun(const FGhostConfig &Config) */
inline Thunk<FGhostRunResponse> postGhostRun(const FGhostConfig &Config) {
  return postGhostRun(
      TypeFactory::GhostRunRequest(Config.TestSuite, Config.Duration));
}

/** User Story: As a api endpoints ghost consumer, I need to invoke get ghost status through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostStatus> getGhostStatus(const FString &SessionId) */
inline Thunk<FGhostStatus> getGhostStatus(const FString &SessionId) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Tags{ghostTagAdapter(SessionId)};
  return Detail::MakeGetWithCodec<FGhostStatus>(
      Data.Names.GetGhostStatus,
      Configuration::endpointPath(
          {Data.Segments.Ghost, SessionId, Data.Segments.Status}),
      Detail::DecodeGhostStatusResponse, Tags);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke get ghost results through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostResults> getGhostResults(const FString &SessionId) */
inline Thunk<FGhostResults> getGhostResults(const FString &SessionId) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Tags{ghostTagAdapter(SessionId)};
  return Detail::MakeGetWithCodec<FGhostResults>(
      Data.Names.GetGhostResults,
      Configuration::endpointPath(
          {Data.Segments.Ghost, SessionId, Data.Segments.Results}),
      Detail::DecodeGhostResultsResponse, Tags);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke post ghost stop through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostStopResponse> postGhostStop(const FString &SessionId) */
inline Thunk<FGhostStopResponse> postGhostStop(const FString &SessionId) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Invalidates{
      ghostTagAdapter(SessionId), ghostListTagAdapter()};
  return Detail::MakePostRawWithCodec<FGhostStopResponse>(
      Data.Names.PostGhostStop,
      Configuration::endpointPath(
          {Data.Segments.Ghost, SessionId, Data.Segments.Stop}),
      Data.Payloads.EmptyObject, Detail::DecodeGhostStopResponse,
      Invalidates);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke get ghost history through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostHistoryResponse> getGhostHistory(int32 Limit) */
inline Thunk<FGhostHistoryResponse> getGhostHistory(int32 Limit) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Tags{ghostListTagAdapter()};
  return Detail::MakeGetWithCodec<FGhostHistoryResponse>(
      Data.Names.GetGhostHistory,
      Configuration::endpointQuery(
          Configuration::endpointPath(
              {Data.Segments.Ghost, Data.Segments.History}),
          Data.QueryKeys.Limit, FString::FromInt(Limit)),
      Detail::DecodeGhostHistoryResponse, Tags);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke get ghost history through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostHistoryResponse> getGhostHistory() */
inline Thunk<FGhostHistoryResponse> getGhostHistory() {
  return getGhostHistory(
      Configuration::endpointData().Defaults.GhostHistoryLimit);
}

} // namespace Endpoints
} // namespace APISlice
