#include "Features/Soul/SoulThunks.h"

#include "Features/API/APIApi.h"
#include "Features/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Errors/ErrorsAdapters.h"
#include "Features/State/StateTypes.h"
#include "Features/Soul/SoulAdapters.h"

namespace rtk {

/** User Story: As a features soul catalog consumer, I need to invoke list souls thunk through a stable signature so the features soul catalog workflow remains explicit and composable. @fn const AsyncThunkConfig<TArray<FSoulListItem>, int32, FRuntimeState> & listSoulsThunk() */
const AsyncThunkConfig<TArray<FSoulListItem>, int32, FRuntimeState> &
listSoulsThunk() {
  const ConditionCallback<int32, FRuntimeState> Condition =
      [](const int32 &, const ThunkApi<FRuntimeState> &Api) {
        return !Api.getState().Soul.bListing;
      };
  static const AsyncThunkConfig<TArray<FSoulListItem>, int32, FRuntimeState>
      Thunk = rtk::createAsyncThunk<TArray<FSoulListItem>, int32, FRuntimeState>(
          TEXT("soul/list"),
          [](const int32 &Limit, const ThunkApi<FRuntimeState> &Api) {
            const int32 ResolvedLimit =
                Limit > 0 ? Limit
                          : APISlice::Endpoints::Configuration::endpointData()
                                .Defaults.SoulListLimit;
            return APISlice::Endpoints::getSoulStorageCatalog(ResolvedLimit)(
                Api.dispatch, Api.getState);
          }, Condition);
  return Thunk;
}

/** User Story: As a features soul catalog consumer, I need to invoke verify soul thunk through a stable signature so the features soul catalog workflow remains explicit and composable. @fn const AsyncThunkConfig<FSoulVerifyResult, FString, FRuntimeState> & verifySoulThunk() */
const AsyncThunkConfig<FSoulVerifyResult, FString, FRuntimeState> &
verifySoulThunk() {
  const ConditionCallback<FString, FRuntimeState> Condition =
      [](const FString &TxId, const ThunkApi<FRuntimeState> &Api) {
        return !TxId.IsEmpty() && !Api.getState().Soul.bVerifying;
      };
  static const AsyncThunkConfig<FSoulVerifyResult, FString, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FSoulVerifyResult, FString, FRuntimeState>(
          TEXT("soul/verify"),
          [](const FString &TxId, const ThunkApi<FRuntimeState> &Api) {
            const func::Maybe<FString> ApiKeyError =
                Errors::requireApiKeyGuidance(SDKConfig::GetApiUrl(),
                                              SDKConfig::GetApiKey());
            return func::match(
                ApiKeyError,
                [](const FString &Error) {
                  return detail::RejectAsync<FSoulVerifyResult>(Error);
                },
                [&TxId, &Api]() {
                  return func::AsyncChain::then<FSoulCatalogEntry,
                                                FSoulVerifyResult>(
                      APISlice::Endpoints::getSoulStorageEntry(TxId)(
                          Api.dispatch, Api.getState),
                      [Api, TxId](const FSoulCatalogEntry &Entry) {
                        return func::AsyncChain::then<FSoulVerifyResult,
                                                      FSoulVerifyResult>(
                            APISlice::Endpoints::postSoulVerification(
                                Entry.TxId,
                                SoulAdapters::
                                    createSoulVerificationRequestAdapter(
                                        Entry))(Api.dispatch, Api.getState),
                            [Api, TxId](const FSoulVerifyResult &Protocol) {
                              return Protocol.bValid
                                         ? APISlice::Endpoints::
                                               getSoulStorageVerification(TxId)(
                                                   Api.dispatch, Api.getState)
                                         : detail::ResolveAsync(Protocol);
                            });
                      });
                });
          }, Condition);
  return Thunk;
}

} // namespace rtk
