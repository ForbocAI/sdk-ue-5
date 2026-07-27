#include "Systems/API/Endpoints/Soul/Provider/SoulProviderAdapters.h"

#include "Async/Async.h"
#include "Core/rtk.hpp"
#include "Systems/API/Transport/Configuration/TransportConfigurationAdapters.h"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Crypto/DataItem/DataItemAdapters.h"
#include "Systems/Soul/Storage/Provider/StorageProviderAdapters.h"
#include "Systems/Soul/Storage/Provider/Response/ResponseAdapters.h"
#include "HAL/PlatformProcess.h"

namespace APISlice::SoulProviderEndpoint {
namespace {

/** User Story: As a Soul provider query, I need synchronous values lifted into the shared FP async channel. @fn template <typename Value> func::AsyncResult<Value> resolvedSoulProviderAdapter(const Value &ValueToResolve) */
template <typename Value>
func::AsyncResult<Value>
resolvedSoulProviderAdapter(const Value &ValueToResolve) {
  return rtk::detail::ResolveAsync(ValueToResolve);
}

/** User Story: As a Soul provider query, I need validation and transport failures lifted into the shared FP async channel. @fn template <typename Value> func::AsyncResult<Value> rejectedSoulProviderAdapter(const FString &Error) */
template <typename Value>
func::AsyncResult<Value> rejectedSoulProviderAdapter(const FString &Error) {
  return rtk::detail::RejectAsync<Value>(Error);
}

/**
 * User Story: As a Soul operator, I need RTK Query errors reduced to their most actionable authored transport detail.
 * @fn FString soulProviderFetchErrorAdapter( const rtk::FetchBaseQueryError &Error)
 */
FString soulProviderFetchErrorAdapter(
    const rtk::FetchBaseQueryError &Error) {
  const SoulStorage::Configuration::FTextData &Text =
      SoulStorage::Configuration::soulStorageData().Text;
  return !Error.error.IsEmpty()
             ? Error.error
             : !Error.data.IsEmpty()
                   ? Error.status + Text.StatusSeparator + Error.data
                   : Error.status;
}

/**
 * User Story: As a Soul operator, I need provider failures retain the operation and RTK Query cause at one error boundary.
 * @fn template <typename Value> FString soulProviderQueryFailureAdapter( const FString &Prefix, const rtk::QueryReturnValue<Value> &Result)
 */
template <typename Value>
FString soulProviderQueryFailureAdapter(
    const FString &Prefix, const rtk::QueryReturnValue<Value> &Result) {
  const FString Cause =
      Result.error.hasValue
          ? soulProviderFetchErrorAdapter(Result.error.value)
          : FString();
  return Cause.IsEmpty()
             ? Prefix
             : Prefix +
                   SoulStorage::Configuration::soulStorageData()
                       .Text.ErrorSeparator +
                   Cause;
}

/** User Story: As a Soul provider endpoint, I need HTTP status read from RTK Query metadata instead of bypassing the transport contract. @fn template <typename Value> func::Maybe<int32> soulProviderStatusAdapter(const rtk::QueryReturnValue<Value> &Result) */
template <typename Value>
func::Maybe<int32>
soulProviderStatusAdapter(const rtk::QueryReturnValue<Value> &Result) {
  return Result.meta.hasValue && Result.meta.value.response.hasValue
             ? func::just(Result.meta.value.response.value.status)
             : func::nothing<int32>();
}

/** User Story: As a provider retry policy, I need delays scheduled off the game thread so RTK Query retries never block runtime work. @fn void scheduleSoulProviderRetryAdapter(int32 DelayMs, const std::function<void()> &Retry) */
void scheduleSoulProviderRetryAdapter(int32 DelayMs,
                                      const std::function<void()> &Retry) {
  AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
            [DelayMs, Retry]() {
              FPlatformProcess::Sleep(static_cast<float>(
                  FTimespan::FromMilliseconds(DelayMs).GetTotalSeconds()));
              AsyncTask(ENamedThreads::GameThread, Retry);
            });
}

/** User Story: As a Soul exporter, I need one immutable data item sent through RTK Query and validated before it can be treated as accepted. @fn func::AsyncResult<FSoulProviderUpload> uploadSoulProviderAdapter(const FSoulPreparedUpload &Pending, const FString &ExpectedTxId) */
func::AsyncResult<FSoulProviderUpload>
uploadSoulProviderAdapter(const FSoulPreparedUpload &Pending,
                          const FString &ExpectedTxId) {
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  rtk::FetchArgs Args;
  Args.url = Data.Provider.UploadUrl;
  Args.method = Data.Provider.UploadMethod;
  Args.body = Pending.DataItem;
  Args.headers.Add(Data.Provider.ContentTypeHeader,
                   Data.Provider.UploadContentType);
  Args.headers.Add(Data.Provider.AcceptHeader, Data.Provider.Accept);
  Args.timeout = Data.Provider.RequestTimeoutMs;
  return func::AsyncChain::then<rtk::QueryReturnValue<FString>,
                                FSoulProviderUpload>(
      rtk::fetchBaseQuery<FString>()(Args, rtk::BaseQueryApi(),
                                    rtk::FEmptyPayload{}),
      [Data, ExpectedTxId](const rtk::QueryReturnValue<FString> &Result) {
        FSoulProviderUploadResponse Response;
        const func::Maybe<int32> Status = soulProviderStatusAdapter(Result);
        const bool bDecoded =
            Result.data.hasValue &&
            SoulStorage::Provider::Response::
                decodeSoulProviderUploadResponseAdapter(Result.data.value,
                                                        Response);
        return Result.error.hasValue
                   ? rejectedSoulProviderAdapter<FSoulProviderUpload>(
                         soulProviderQueryFailureAdapter(Data.Text.UploadFailed,
                                                         Result))
                   : !bDecoded
                         ? rejectedSoulProviderAdapter<FSoulProviderUpload>(
                               Data.Text.UploadResponseInvalid)
                         : Response.Id != ExpectedTxId
                               ? rejectedSoulProviderAdapter<
                                     FSoulProviderUpload>(
                                     Data.Text.TransactionMismatch)
                               : func::is_nothing(Status)
                                     ? rejectedSoulProviderAdapter<
                                           FSoulProviderUpload>(
                                           Data.Text.ProviderStatusMissing)
                                     : resolvedSoulProviderAdapter(
                                           FSoulProviderUpload{Response,
                                                               Status.value});
      });
}

/**
 * User Story: As a Soul exporter, I need failed upload attempts retried independently from retrieval so accepted transactions are submitted once.
 * @fn void executeSoulProviderUploadAttemptAdapter( const FSoulPreparedUpload &Pending, const FString &ExpectedTxId, int32 Attempt, const std::function<void(FSoulProviderUpload)> &Resolve, const std::function<void(std::string)> &Reject)
 */
void executeSoulProviderUploadAttemptAdapter(
    const FSoulPreparedUpload &Pending, const FString &ExpectedTxId,
    int32 Attempt,
    const std::function<void(FSoulProviderUpload)> &Resolve,
    const std::function<void(std::string)> &Reject) {
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  uploadSoulProviderAdapter(Pending, ExpectedTxId)
      .then(Resolve)
      .catch_([Pending, ExpectedTxId, Attempt, Resolve, Reject,
               Data](const std::string &Error) {
        const int32 NextAttempt = Attempt + Data.Upload.AttemptStep;
        NextAttempt >= Data.Upload.MaximumAttempts
            ? Reject(Error)
            : scheduleSoulProviderRetryAdapter(
                  Data.Upload.DelayMs,
                  [Pending, ExpectedTxId, NextAttempt, Resolve, Reject]() {
                    executeSoulProviderUploadAttemptAdapter(
                        Pending, ExpectedTxId, NextAttempt, Resolve, Reject);
                  });
      })
      .execute();
}

/** User Story: As a Soul importer, I need one provider URL fetched through binary RTK Query transport before digest validation. @fn func::AsyncResult<FSoulProviderPayload> readSoulProviderPayloadAdapter(const FString &Url, const FString &Digest) */
func::AsyncResult<FSoulProviderPayload>
readSoulProviderPayloadAdapter(const FString &Url, const FString &Digest) {
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  rtk::FetchBaseQueryArgs Options;
  Options.responseHandler = FString();
  rtk::FetchArgs Args;
  Args.url = Url;
  Args.method = Transport::transportQueryData().Methods.Get;
  Args.timeout = Data.Provider.RequestTimeoutMs;
  return func::AsyncChain::then<rtk::QueryReturnValue<TArray<uint8>>,
                                FSoulProviderPayload>(
      rtk::fetchBaseQuery<TArray<uint8>>(Options)(
          Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{}),
      [Data, Url, Digest](
          const rtk::QueryReturnValue<TArray<uint8>> &Result) {
        FString DigestError;
        const FString RetrievedDigest =
            Result.data.hasValue
                ? SoulStorage::Crypto::DataItem::soulPayloadDigestAdapter(
                      Result.data.value, DigestError)
                : FString();
        return Result.error.hasValue
                   ? rejectedSoulProviderAdapter<FSoulProviderPayload>(
                         soulProviderQueryFailureAdapter(
                             Data.Text.RetrievalFailed +
                                 Data.Text.RetrievalUrlSeparator + Url,
                             Result))
                   : !Result.data.hasValue
                         ? rejectedSoulProviderAdapter<FSoulProviderPayload>(
                               Data.Text.RetrievalNetworkFailed +
                               Data.Text.RetrievalUrlSeparator + Url)
                         : !DigestError.IsEmpty()
                               ? rejectedSoulProviderAdapter<
                                     FSoulProviderPayload>(DigestError)
                               : RetrievedDigest != Digest
                                     ? rejectedSoulProviderAdapter<
                                           FSoulProviderPayload>(
                                           Data.Text.RetrievalMismatch)
                                     : resolvedSoulProviderAdapter(
                                           FSoulProviderPayload{
                                               Result.data.value, Url});
      });
}

/**
 * User Story: As a Soul verifier, I need each provider URL attempted through RTK Query and delayed only after a complete gateway cycle.
 * @fn void executeSoulProviderRetrievalAttemptAdapter( const TArray<FString> &Urls, const FString &Digest, int32 Attempt, const std::function<void(FSoulProviderPayload)> &Resolve, const std::function<void(std::string)> &Reject)
 */
void executeSoulProviderRetrievalAttemptAdapter(
    const TArray<FString> &Urls, const FString &Digest, int32 Attempt,
    const std::function<void(FSoulProviderPayload)> &Resolve,
    const std::function<void(std::string)> &Reject) {
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  readSoulProviderPayloadAdapter(Urls[Attempt % Urls.Num()], Digest)
      .then(Resolve)
      .catch_([Urls, Digest, Attempt, Resolve, Reject,
               Data](const std::string &Error) {
        const int32 NextAttempt = Attempt + Data.Retrieval.AttemptStep;
        SoulStorage::Provider::soulProviderRetryExhaustedAdapter(
            NextAttempt, Urls.Num())
            ? Reject(Error)
            : scheduleSoulProviderRetryAdapter(
                  SoulStorage::Provider::soulProviderRetryDelayAdapter(
                      NextAttempt, Urls.Num()),
                  [Urls, Digest, NextAttempt, Resolve, Reject]() {
                    executeSoulProviderRetrievalAttemptAdapter(
                        Urls, Digest, NextAttempt, Resolve, Reject);
                  });
      })
      .execute();
}

} // namespace

/** User Story: As a Soul exporter, I need only failed provider uploads retried so an accepted immutable transaction is never resubmitted during retrieval polling. @fn func::AsyncResult<FSoulProviderUpload> uploadSoulProviderWithRetryAdapter(const FSoulPreparedUpload &Pending, const FString &ExpectedTxId, int32 Attempt) */
func::AsyncResult<FSoulProviderUpload>
uploadSoulProviderWithRetryAdapter(const FSoulPreparedUpload &Pending,
                                   const FString &ExpectedTxId,
                                   int32 Attempt) {
  return func::AsyncResult<FSoulProviderUpload>::create(
      [Pending, ExpectedTxId, Attempt](
          std::function<void(FSoulProviderUpload)> Resolve,
          std::function<void(std::string)> Reject) {
        executeSoulProviderUploadAttemptAdapter(
            Pending, ExpectedTxId, Attempt, Resolve, Reject);
      });
}

/** User Story: As a Soul verifier, I need provider gateways rotated through RTK Query until one returns bytes matching the authorized digest. @fn func::AsyncResult<FSoulProviderPayload> retrieveMatchingSoulPayloadAdapter(const TArray<FString> &Urls, const FString &Digest, int32 Attempt) */
func::AsyncResult<FSoulProviderPayload>
retrieveMatchingSoulPayloadAdapter(const TArray<FString> &Urls,
                                   const FString &Digest, int32 Attempt) {
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  return Urls.IsEmpty()
             ? rejectedSoulProviderAdapter<FSoulProviderPayload>(
                   Data.Text.RetrievalFailed)
             : func::AsyncResult<FSoulProviderPayload>::create(
                   [Urls, Digest, Attempt](
                       std::function<void(FSoulProviderPayload)> Resolve,
                       std::function<void(std::string)> Reject) {
                     executeSoulProviderRetrievalAttemptAdapter(
                         Urls, Digest, Attempt, Resolve, Reject);
                   });
}

} // namespace APISlice::SoulProviderEndpoint
