#pragma once

#include "Systems/API/APIAdapters.h"
#include "Components/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints system consumer, I need to invoke get api status through a stable signature so the api endpoints system workflow remains explicit and composable. @fn inline Thunk<FApiStatusResponse> getApiStatus() */
inline Thunk<FApiStatusResponse> getApiStatus() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakeGet<FApiStatusResponse>(
      Data.Names.GetApiStatus,
      Configuration::endpointPath({Data.Segments.Status}),
      TArray<rtk::FApiEndpointTag>{diagnosticsTagAdapter()},
      Data.Timeouts.StatusMs);
}

/** User Story: As an SDK verifier, I need the API-owned test contract fetched through the SDK API cache. @fn inline Thunk<FString> getMicroGameContract() */
inline Thunk<FString> getMicroGameContract() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakeGet<FString>(
      Data.Names.GetMicroGameContract,
      Configuration::endpointPath(
          {Data.Segments.MicroGame, Data.Segments.Contract}),
      TArray<rtk::FApiEndpointTag>{diagnosticsTagAdapter()},
      Data.Timeouts.ContractMs);
}

} // namespace Endpoints
} // namespace APISlice
