#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints bridge consumer, I need to invoke get bridge validation through a stable signature so the api endpoints bridge workflow remains explicit and composable. @fn inline Thunk<FValidationResult> getBridgeValidation(const FString &NpcId, const FBridgeValidateRequest &Request) */
inline Thunk<FValidationResult>
getBridgeValidation(const FString &NpcId,
                    const FBridgeValidateRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const FString Path = NpcId.IsEmpty()
                           ? Configuration::endpointPath(
                                 {Data.Segments.Bridge, Data.Segments.Validate})
                           : Configuration::endpointPath(
                                 {Data.Segments.Bridge, Data.Segments.Validate,
                                  NpcId});
  const TArray<FApiEndpointTag> Tags{bridgeTagAdapter(NpcId)};
  return Detail::MakePostQueryWithCodec<FBridgeValidateRequest,
                                        FValidationResult>(
      Data.Names.GetBridgeValidation, Configuration::apiEndpoint(Path),
      Request, Detail::EncodeBridgeValidateRequest,
      Detail::DecodeValidationResult, Tags);
}

/** User Story: As a api endpoints bridge consumer, I need to invoke get bridge rules through a stable signature so the api endpoints bridge workflow remains explicit and composable. @fn inline Thunk<TArray<FBridgeRule>> getBridgeRules() */
inline Thunk<TArray<FBridgeRule>> getBridgeRules() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<FApiEndpointTag> Tags{bridgeListTagAdapter()};
  return Detail::MakeGetWithCodec<TArray<FBridgeRule>>(
      Data.Names.GetBridgeRules,
      Configuration::apiEndpoint(Configuration::endpointPath(
          {Data.Segments.Bridge, Data.Segments.Rules})),
      Detail::DecodeBridgeRulesResponse, Tags);
}

} // namespace Endpoints
} // namespace APISlice
