#include "Features/Soul/SoulThunks.h"

#include "Features/API/APIApi.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Errors/ErrorsAdapters.h"
#include "Features/Runtime/RuntimeTypes.h"
#include "Features/Soul/SoulAdapters.h"
#include "Features/Soul/SoulSelectors.h"
#include "Features/Soul/Storage/Configuration/ConfigurationAdapters.h"

namespace {

/**
 * User Story: As a Soul importer, I need API authorization completed before
 * provider retrieval and authenticated decryption begin.
 * @fn func::AsyncResult<FSoul> loadVerifiedSoulThunk(const FString &TxId, const rtk::ThunkApi<FRuntimeState> &Api)
 */
func::AsyncResult<FSoul>
loadVerifiedSoulThunk(const FString &TxId,
                      const rtk::ThunkApi<FRuntimeState> &Api) {
  return func::AsyncChain::then<FSoulCatalogEntry, FSoul>(
      APISlice::Endpoints::getSoulStorageEntry(TxId)(Api.dispatch,
                                                     Api.getState),
      [Api, TxId](const FSoulCatalogEntry &Entry) {
        return func::AsyncChain::then<FSoulVerifyResult, FSoul>(
            APISlice::Endpoints::postSoulVerification(
                Entry.TxId,
                SoulAdapters::createSoulVerificationRequestAdapter(Entry))(
                Api.dispatch, Api.getState),
            [Api, TxId](const FSoulVerifyResult &Verification) {
              return Verification.bValid
                         ? APISlice::Endpoints::getSoulStorageDownload(TxId)(
                               Api.dispatch, Api.getState)
                         : rtk::detail::RejectAsync<FSoul>(
                               Verification.Reason.IsEmpty()
                                   ? SoulStorage::Configuration::
                                         soulStorageData()
                                             .Text.VerificationFailed
                                   : Verification.Reason);
            });
      });
}

} // namespace

namespace rtk {

/** User Story: As a features soul import consumer, I need to invoke import soul thunk through a stable signature so the features soul import workflow remains explicit and composable. @fn const AsyncThunkConfig<FSoul, FString, FRuntimeState> &importSoulThunk() */
const AsyncThunkConfig<FSoul, FString, FRuntimeState> &importSoulThunk() {
  const ConditionCallback<FString, FRuntimeState> Condition =
      [](const FString &TxId, const ThunkApi<FRuntimeState> &Api) {
        return !TxId.IsEmpty() &&
               SoulSelectors::selectSoulImportStatus(Api.getState().Soul) !=
                   SoulStorage::Configuration::soulStorageData()
                       .Lifecycle.Importing;
      };
  static const AsyncThunkConfig<FSoul, FString, FRuntimeState> Thunk =
      rtk::createAsyncThunk<FSoul, FString, FRuntimeState>(
          TEXT("soul/import"),
          [](const FString &TxId, const ThunkApi<FRuntimeState> &Api) {
            const func::Maybe<FString> ApiKeyError =
                Errors::requireApiKeyGuidance(SDKConfig::GetApiUrl(),
                                              SDKConfig::GetApiKey());
            return ApiKeyError.hasValue
                       ? detail::RejectAsync<FSoul>(ApiKeyError.value)
                       : loadVerifiedSoulThunk(TxId, Api);
          }, Condition);
  return Thunk;
}

} // namespace rtk
