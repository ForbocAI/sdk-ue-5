#pragma once

#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<FDirectiveRuleSet> postBridgePreset(const FString &PresetName) {
  return Detail::MakePostRawWithCodec<FDirectiveRuleSet>(
      TEXT("postBridgePreset"),
      SDKConfig::GetApiUrl() + TEXT("/rules/presets/") +
          Detail::Encode(PresetName),
      TEXT("{}"), Detail::DecodeDirectiveRuleSetResponse);
}

inline Thunk<TArray<FDirectiveRuleSet>> getRulesets() {
  return Detail::MakeGetWithCodec<TArray<FDirectiveRuleSet>>(
      TEXT("getRulesets"), SDKConfig::GetApiUrl() + TEXT("/rules"),
      Detail::DecodeDirectiveRuleSetListResponse);
}

inline Thunk<TArray<FString>> getRulePresets() {
  return Detail::MakeGet<TArray<FString>>(
      TEXT("getRulePresets"), SDKConfig::GetApiUrl() + TEXT("/rules/presets"));
}

inline Thunk<FDirectiveRuleSet>
postRuleRegister(const FDirectiveRuleSet &Ruleset) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Rule"), TEXT("LIST")});
  return Detail::MakePostWithCodec<FDirectiveRuleSet, FDirectiveRuleSet>(
      TEXT("postRuleRegister"), SDKConfig::GetApiUrl() + TEXT("/rules"),
      Ruleset, Detail::ToJson<FDirectiveRuleSet>,
      Detail::DecodeDirectiveRuleSetResponse, Invalidates);
}

inline Thunk<rtk::FEmptyPayload> deleteRule(const FString &RulesetId) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Rule"), TEXT("LIST")});
  return Detail::MakeDelete<rtk::FEmptyPayload>(
      TEXT("deleteRule"),
      SDKConfig::GetApiUrl() + TEXT("/rules/") + Detail::Encode(RulesetId),
      Invalidates);
}

} // namespace Endpoints
} // namespace APISlice
