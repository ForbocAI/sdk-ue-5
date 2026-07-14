#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<FValidationResult> postBridgeValidate(const FString &NpcId,
                                                   const FString &PayloadJson) {
  const FString Url = NpcId.IsEmpty()
                          ? SDKConfig::GetApiUrl() + TEXT("/bridge/validate")
                          : SDKConfig::GetApiUrl() + TEXT("/bridge/validate/") +
                                Detail::Encode(NpcId);
  return Detail::MakePostRawWithCodec<FValidationResult>(
      TEXT("postBridgeValidate"), Url, PayloadJson,
      Detail::DecodeValidationResult);
}

inline Thunk<FValidationResult>
postBridgeValidate(const FString &NpcId,
                   const FBridgeValidateRequest &Request) {
  const FString Url = NpcId.IsEmpty()
                          ? SDKConfig::GetApiUrl() + TEXT("/bridge/validate")
                          : SDKConfig::GetApiUrl() + TEXT("/bridge/validate/") +
                                Detail::Encode(NpcId);
  return Detail::MakePostWithCodec<FBridgeValidateRequest, FValidationResult>(
      TEXT("postBridgeValidate"), Url, Request,
      Detail::EncodeBridgeValidateRequest, Detail::DecodeValidationResult);
}

inline Thunk<TArray<FBridgeRule>> getBridgeRules() {
  return Detail::MakeGetWithCodec<TArray<FBridgeRule>>(
      TEXT("getBridgeRules"), SDKConfig::GetApiUrl() + TEXT("/bridge/rules"),
      Detail::DecodeBridgeRulesResponse);
}

} // namespace Endpoints
} // namespace APISlice
