#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "Errors.h"
#include "Features/API/APIApi.h"
#include "Features/Soul/SoulSlice.h"
#include "RuntimeConfig.h"

namespace rtk {

inline ThunkAction<TArray<FSoulListItem>, FRuntimeState>
getSoulListThunk(int32 Limit = 50) {
  return [Limit](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FSoulListItem>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<TArray<FSoulListItem>>(ApiKeyError.value)
        : func::AsyncChain::then<FSoulListResponse, TArray<FSoulListItem>>(
              APISlice::Endpoints::getSouls(Limit)(Dispatch, GetState),
              [Dispatch](const FSoulListResponse &Response) {
                Dispatch(SoulSlice::Actions::setSoulList(Response.Souls));
                return detail::ResolveAsync(Response.Souls);
              });
  };
}

inline ThunkAction<FSoulVerifyResult, FRuntimeState>
verifySoulThunk(const FString &TxId) {
  return [TxId](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoulVerifyResult> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FSoulVerifyResult>(ApiKeyError.value)
        : APISlice::Endpoints::postSoulVerify(TxId)(Dispatch, GetState);
  };
}

} // namespace rtk
