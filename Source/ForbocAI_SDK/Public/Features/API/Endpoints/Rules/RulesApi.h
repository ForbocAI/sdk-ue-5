#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

/** User Story: As a api endpoints rules consumer, I need to invoke post bridge preset through a stable signature so the api endpoints rules workflow remains explicit and composable. @fn inline Thunk<FDirectiveRuleSet> postBridgePreset(const FString &PresetName) */
inline Thunk<FDirectiveRuleSet> postBridgePreset(const FString &PresetName) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Invalidates{
      bridgeListTagAdapter(), ruleListTagAdapter()};
  return Detail::MakePostRawWithCodec<FDirectiveRuleSet>(
      Data.Names.PostBridgePreset,
      Configuration::endpointPath(
          {Data.Segments.Rules, Data.Segments.Presets, PresetName}),
      Data.Payloads.EmptyObject, Detail::DecodeDirectiveRuleSetResponse,
      Invalidates);
}

/** User Story: As a api endpoints rules consumer, I need to invoke get rulesets through a stable signature so the api endpoints rules workflow remains explicit and composable. @fn inline Thunk<TArray<FDirectiveRuleSet>> getRulesets() */
inline Thunk<TArray<FDirectiveRuleSet>> getRulesets() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Tags{ruleListTagAdapter()};
  return Detail::MakeGetWithCodec<TArray<FDirectiveRuleSet>>(
      Data.Names.GetRulesets,
      Configuration::endpointPath({Data.Segments.Rules}),
      Detail::DecodeDirectiveRuleSetListResponse, Tags);
}

/** User Story: As a api endpoints rules consumer, I need to invoke get rule presets through a stable signature so the api endpoints rules workflow remains explicit and composable. @fn inline Thunk<TArray<FString>> getRulePresets() */
inline Thunk<TArray<FString>> getRulePresets() {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const TArray<rtk::FApiEndpointTag> Tags{ruleListTagAdapter()};
  return Detail::MakeGet<TArray<FString>>(
      Data.Names.GetRulePresets,
      Configuration::endpointPath(
          {Data.Segments.Rules, Data.Segments.Presets}),
      Tags);
}

} // namespace Endpoints
} // namespace APISlice
