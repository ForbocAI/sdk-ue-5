#pragma once

#include "Systems/API/APIAdapters.h"
#include "Systems/Ghost/GhostAdapters.h"
#include "Components/API/Endpoints/EndpointsTypes.h"
#include "Components/NPC/Generate/GenerateTypes.h"
#include "Systems/API/Endpoints/NPC/Generate/GenerateAdapters.h"
#include "Systems/API/Endpoints/NPC/Process/NPCProcessAdapters.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints ghost consumer, I need to invoke post ghost run through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostRunResponse> postGhostRun(const FGhostRunRequest &Request) */
inline Thunk<FGhostRunResponse> postGhostRun(const FGhostRunRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Invalidates{ghostListTagAdapter()};
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

/** User Story: As Ghost cognition, I need the shared process tape sent only through the active Ghost session endpoint. @fn inline Thunk<FNPCProcessResponse> postGhostProcess(const FString &SessionId, const FNPCProcessRequest &Request) */
inline Thunk<FNPCProcessResponse>
postGhostProcess(const FString &SessionId,
                 const FNPCProcessRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Invalidates{
      ghostTagAdapter(SessionId)};
  return Detail::MakePostWithCodec<FNPCProcessRequest, FNPCProcessResponse>(
      Data.Names.PostGhostProcess,
      Configuration::endpointPath(
          {Data.Segments.Ghost, SessionId, Data.Segments.Process}),
      Request, Detail::EncodeNpcProcessRequest,
      Detail::DecodeNpcProcessResponse, Invalidates,
      Data.Timeouts.NpcProcessMs);
}

/**
 * User Story: As Ghost actor construction, I need one generated attribute routed only through the active Ghost session endpoint.
 * @fn inline Thunk<FNpcAttributeGenerateResponse> postGhostNpcGenerateAttribute( const FString &SessionId, const FString &Attribute, const FNpcAttributeGenerateRequest &Request)
 */
inline Thunk<FNpcAttributeGenerateResponse> postGhostNpcGenerateAttribute(
    const FString &SessionId, const FString &Attribute,
    const FNpcAttributeGenerateRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Invalidates{
      ghostTagAdapter(SessionId)};
  return Detail::MakePostWithCodec<FNpcAttributeGenerateRequest,
                                   FNpcAttributeGenerateResponse>(
      Data.Names.PostGhostNpcGenerateAttribute,
      Configuration::endpointPath(
          {Data.Segments.Ghost, SessionId, Data.Segments.Npcs,
           Data.Segments.Generate, Attribute}),
      Request, Detail::EncodeNpcAttributeGenerateRequest,
      Detail::DecodeNpcAttributeGenerateResponse, Invalidates,
      Data.Timeouts.NpcGenerateMs);
}

/** User Story: As a api endpoints ghost consumer, I need to invoke get ghost status through a stable signature so the api endpoints ghost workflow remains explicit and composable. @fn inline Thunk<FGhostStatus> getGhostStatus(const FString &SessionId) */
inline Thunk<FGhostStatus> getGhostStatus(const FString &SessionId) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Tags{ghostTagAdapter(SessionId)};
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
  const TArray<rtk::FApiEndpointTag> Tags{ghostTagAdapter(SessionId)};
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
  const TArray<rtk::FApiEndpointTag> Invalidates{
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
  const TArray<rtk::FApiEndpointTag> Tags{ghostListTagAdapter()};
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
