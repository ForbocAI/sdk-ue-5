#include "Systems/Soul/SoulThunks.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/API/APIApi.h"
#include "Systems/API/Serialization/Agent/AgentAdapters.h"
#include "Entities/Config/ConfigSelectors.h"
#include "Systems/Errors/ErrorsAdapters.h"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Local/MemoryLocalThunks.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Components/State/StateTypes.h"
#include "Systems/Soul/SoulAdapters.h"
#include "Entities/Soul/SoulSelectors.h"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"

namespace rtk {
namespace {

/**
 * User Story: As a failed Soul export, I need prepared bytes removed before the original failure leaves the thunk.
 * @fn func::AsyncResult<FSoulExportResult> cleanupSoulExportThunk( const func::AsyncResult<FSoulExportResult> &Pipeline, const std::shared_ptr<FString> &PreparedTxId, const ThunkApi<FRuntimeState> &Api)
 */
func::AsyncResult<FSoulExportResult> cleanupSoulExportThunk(
    const func::AsyncResult<FSoulExportResult> &Pipeline,
    const std::shared_ptr<FString> &PreparedTxId,
    const ThunkApi<FRuntimeState> &Api) {
  return func::AsyncResult<FSoulExportResult>::create(
      [Pipeline, PreparedTxId, Api](
          std::function<void(FSoulExportResult)> Resolve,
          std::function<void(std::string)> Reject) {
        Pipeline
            .then([Resolve](const FSoulExportResult &Result) {
              Resolve(Result);
            })
            .catch_([PreparedTxId, Api, Reject](const std::string &Failure) {
              const FString Message = Errors::extractThunkErrorMessage(
                  Failure, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFD0B1165D035));
              PreparedTxId->IsEmpty()
                  ? Reject(std::string(TCHAR_TO_UTF8(*Message)))
                  : APISlice::Endpoints::deleteSoulStoragePreparation(
                        *PreparedTxId)(Api.dispatch, Api.getState)
                        .then([Message, Reject](const FEmptyPayload &) {
                          Reject(std::string(TCHAR_TO_UTF8(*Message)));
                        })
                        .catch_([Message, Reject](
                                    const std::string &CleanupFailure) {
                          const FString CleanupMessage =
                              Errors::extractThunkErrorMessage(CleanupFailure);
                          const FString Combined =
                              Message +
                              SoulStorage::Configuration::soulStorageData()
                                  .Text.CleanupFailedSeparator +
                              CleanupMessage;
                          Reject(std::string(TCHAR_TO_UTF8(*Combined)));
                        })
                        .execute();
            })
            .execute();
      });
}

/**
 * User Story: As a Soul exporter, I need local encrypted bytes bound to API authorization before provider upload.
 * @fn func::AsyncResult<FSoulExportResult> executeSoulExportThunk( const FSoul &Soul, const FString &NpcId, const ThunkApi<FRuntimeState> &Api)
 */
func::AsyncResult<FSoulExportResult> executeSoulExportThunk(
    const FSoul &Soul, const FString &NpcId,
    const ThunkApi<FRuntimeState> &Api) {
  const std::shared_ptr<FString> PreparedTxId =
      std::make_shared<FString>();
  const func::AsyncResult<FSoulExportResult> Pipeline =
      func::AsyncChain::then<FSoulStoragePreparation, FSoulExportResult>(
          APISlice::Endpoints::postSoulStoragePreparation(Soul)(Api.dispatch,
                                                                Api.getState),
          [Soul, NpcId, Api,
           PreparedTxId](const FSoulStoragePreparation &LocalPreparation) {
            *PreparedTxId = LocalPreparation.TxId;
            FSoulExportPreparationRequest PreparationRequest;
            PreparationRequest.NpcIdRef = NpcId;
            PreparationRequest.TransactionId = LocalPreparation.TxId;
            PreparationRequest.PayloadDigest = LocalPreparation.Digest;
            PreparationRequest.Provider = LocalPreparation.Provider;
            return func::AsyncChain::then<FSoulExportPreparation,
                                          FSoulExportResult>(
                APISlice::Endpoints::postSoulExportPreparation(
                    NpcId, PreparationRequest)(Api.dispatch, Api.getState),
                [Soul, NpcId, Api, LocalPreparation](
                    const FSoulExportPreparation &RemotePreparation) {
                  return !SoulAdapters::matchesSoulExportPreparationAdapter(
                             LocalPreparation, RemotePreparation, NpcId)
                             ? detail::RejectAsync<FSoulExportResult>(
                                   SoulStorage::Configuration::
                                       soulStorageData()
                                           .Text.ConfirmationMismatch)
                             : func::AsyncChain::then<FSoulStorageReceipt,
                                                      FSoulExportResult>(
                                   APISlice::Endpoints::postSoulStorageUpload(
                                       LocalPreparation.TxId)(Api.dispatch,
                                                              Api.getState),
                                   [Soul, NpcId, Api, RemotePreparation](
                                       const FSoulStorageReceipt &Receipt) {
                                     FSoulExportConfirmationRequest
                                         ConfirmationRequest;
                                     ConfirmationRequest.NpcIdRef =
                                         RemotePreparation.NpcIdRef;
                                     ConfirmationRequest.TransactionId =
                                         RemotePreparation.TransactionId;
                                     ConfirmationRequest.PayloadDigest =
                                         RemotePreparation.PayloadDigest;
                                     ConfirmationRequest.Provider =
                                         RemotePreparation.Provider;
                                     ConfirmationRequest.ConfirmationToken =
                                         RemotePreparation.ConfirmationToken;
                                     ConfirmationRequest.StorageUrl =
                                         Receipt.Url;
                                     ConfirmationRequest.ProviderStatus =
                                         Receipt.Status;
                                     return func::AsyncChain::then<
                                         FSoulExportResponse,
                                         FSoulExportResult>(
                                         APISlice::Endpoints::
                                             postSoulExportConfirmation(
                                                 NpcId, ConfirmationRequest)(
                                                 Api.dispatch, Api.getState),
                                         [Soul, Api, Receipt](
                                             const FSoulExportResponse
                                                 &Confirmation) {
                                           return !SoulAdapters::
                                                       matchesSoulExportConfirmationAdapter(
                                                           Receipt,
                                                           Confirmation)
                                                      ? detail::RejectAsync<
                                                            FSoulExportResult>(
                                                            SoulStorage::
                                                                Configuration::
                                                                    soulStorageData()
                                                                        .Text
                                                                        .ConfirmationMismatch)
                                                      : [&]() {
                                                          FSoulStorageCommit
                                                              Commit;
                                                          Commit.Soul = Soul;
                                                          Commit.Receipt =
                                                              Receipt;
                                                          Commit.Signature =
                                                              Confirmation
                                                                  .Signature;
                                                          return func::
                                                              AsyncChain::then<
                                                                  FSoulCatalogEntry,
                                                                  FSoulExportResult>(
                                                                  APISlice::
                                                                      Endpoints::
                                                                          postSoulStorageCommit(
                                                                              Commit)(
                                                                              Api.dispatch,
                                                                              Api.getState),
                                                                  [Soul,
                                                                   Confirmation](
                                                                      const FSoulCatalogEntry &) {
                                                                    FSoulExportResult
                                                                        Result;
                                                                    Result.TxId =
                                                                        Confirmation
                                                                            .TxId;
                                                                    Result.Url =
                                                                        Confirmation
                                                                            .StorageUrl;
                                                                    Result.Soul =
                                                                        Soul;
                                                                    return detail::
                                                                        ResolveAsync(
                                                                            Result);
                                                                  });
                                                        }();
                                         });
                                   });
                });
          });
  return cleanupSoulExportThunk(Pipeline, PreparedTxId, Api);
}

} // namespace

/** User Story: As a features soul export consumer, I need to invoke export soul thunk through a stable signature so the features soul export workflow remains explicit and composable. @fn const AsyncThunkConfig<FSoulExportResult, FString, FRuntimeState> & exportSoulThunk() */
const AsyncThunkConfig<FSoulExportResult, FString, FRuntimeState> &
exportSoulThunk() {
  const ConditionCallback<FString, FRuntimeState> Condition =
      [](const FString &, const ThunkApi<FRuntimeState> &Api) {
        return SoulSelectors::selectSoulExportStatus(Api.getState().Soul) !=
               SoulStorage::Configuration::soulStorageData()
                   .Lifecycle.Exporting;
      };
  static const AsyncThunkConfig<FSoulExportResult, FString, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FSoulExportResult, FString, FRuntimeState>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD533C38073B6),
          [](const FString &RequestedNpcId,
             const ThunkApi<FRuntimeState> &Api) {
            const func::Maybe<FString> ApiKeyError =
                Errors::requireApiKeyGuidance(
                    ConfigSelectors::selectApiUrl(Api.getState()),
                    ConfigSelectors::selectApiKey(Api.getState()));
            return func::match(
                ApiKeyError,
                [](const FString &Error) {
                  return detail::RejectAsync<FSoulExportResult>(Error);
                },
                [&RequestedNpcId, &Api]() {
                  const FString NpcId =
                      RequestedNpcId.IsEmpty()
                          ? NPCSelectors::selectActiveNpcId(
                                Api.getState().NPCs)
                          : RequestedNpcId;
                  const func::Maybe<FNPCInternalState> Npc =
                      NPCSelectors::selectNPCById(Api.getState().NPCs, NpcId);
                  return func::match(
                      Npc,
                      [&NpcId, &Api](const FNPCInternalState &NpcState) {
                        const MemoryConfiguration::FMemoryData &MemoryData =
                            MemoryConfiguration::memoryData();
                        return func::AsyncChain::then<
                            TArray<FMemoryItem>, FSoulExportResult>(
                            listNodeMemoryThunk(
                                MemoryData.Defaults.ListLimit,
                                MemoryData.Defaults.ListOffset,
                                NpcId)(Api.dispatch, Api.getState),
                            [NpcId, Api, NpcState](
                                const TArray<FMemoryItem> &Memories) {
                              const SoulStorage::Configuration::FSoulData
                                  &SoulData =
                                      SoulStorage::Configuration::
                                          soulStorageData()
                                              .Soul;
                              const FSoul Soul =
                                  SoulAdapters::createSoulAdapter(
                                      NpcId,
                                      JsonInterop::StringifyObject(
                                          JsonInterop::
                                              StructuredPersonaToObject(
                                                  NpcState.Persona)),
                                      NpcState.State, Memories,
                                      SoulData.DefaultName,
                                      SoulData.Version);
                              return executeSoulExportThunk(
                                  Soul, NpcId, Api);
                            });
                      },
                      [&NpcId]() {
                        return detail::RejectAsync<FSoulExportResult>(
                            FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3606A5269ADB), *NpcId));
                      });
                });
          }, Condition);
  return Thunk;
}

} // namespace rtk
