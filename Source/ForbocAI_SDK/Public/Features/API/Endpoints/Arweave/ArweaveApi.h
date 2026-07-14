#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsAdapters.h"

#include "Async/Async.h"
#include "HAL/PlatformProcess.h"

namespace APISlice {

namespace Detail {

template <typename Result>
inline func::AsyncResult<rtk::QueryReturnValue<Result>>
QueryFailure(const FString &Message) {
  return func::AsyncResult<rtk::QueryReturnValue<Result>>::create(
      [Message](std::function<void(rtk::QueryReturnValue<Result>)> Resolve,
                std::function<void(std::string)> Reject) {
        (void)Reject;
        Resolve(rtk::QueryReturnValue<Result>::failure(
            rtk::FetchBaseQueryError::fetchError(Message)));
      });
}

inline func::AsyncResult<rtk::QueryReturnValue<FString>>
ExecuteArweaveBaseQuery(const rtk::FetchArgs &Args) {
  rtk::FetchBaseQueryArgs Options;
  Options.responseHandler = TEXT("text");
  return rtk::fetchBaseQuery<FString>(Options)(
      Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{});
}

inline func::AsyncResult<rtk::QueryReturnValue<FArweaveUploadResult>>
ExecuteArweaveUploadAttempt(const FArweaveUploadRequest &Request,
                            int32 Attempt);

inline void ScheduleArweaveUploadRetry(
    const FArweaveUploadRequest &Request, int32 Attempt,
    std::function<void(rtk::QueryReturnValue<FArweaveUploadResult>)> Resolve,
    std::function<void(std::string)> Reject) {
  const float BackoffSec =
      0.250f * FMath::Pow(2.0f, static_cast<float>(Attempt));
  AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
            [Request, Attempt, Resolve, Reject, BackoffSec]() {
              FPlatformProcess::Sleep(BackoffSec);
              AsyncTask(ENamedThreads::GameThread,
                        [Request, Attempt, Resolve, Reject]() {
                          ExecuteArweaveUploadAttempt(Request, Attempt)
                              .then(Resolve)
                              .catch_(Reject)
                              .execute();
                        });
            });
}

inline func::AsyncResult<rtk::QueryReturnValue<FArweaveUploadResult>>
ExecuteArweaveUploadAttempt(const FArweaveUploadRequest &Request,
                            int32 Attempt) {
  const FString Url = ArweaveUploadUrl(Request.Instruction);
  return Url.IsEmpty()
             ? QueryFailure<FArweaveUploadResult>(
                   TEXT("Missing Arweave upload URL"))
             : func::AsyncResult<rtk::QueryReturnValue<
                   FArweaveUploadResult>>::create(
                   [Request, Attempt](
                       std::function<void(
                           rtk::QueryReturnValue<FArweaveUploadResult>)>
                           Resolve,
                       std::function<void(std::string)> Reject) {
                     ExecuteArweaveBaseQuery(ArweaveUploadFetchArgs(Request))
                         .then([Request, Attempt, Resolve, Reject](
                                   const rtk::QueryReturnValue<FString>
                                       &QueryResult) {
                           const FArweaveUploadResult Result =
                               UploadResultFromQuery(QueryResult,
                                                     Request.Instruction);
                           !Result.bSuccess &&
                                   Attempt + 1 < Request.MaxRetries
                               ? ScheduleArweaveUploadRetry(
                                     Request, Attempt + 1, Resolve, Reject)
                               : Resolve(rtk::QueryReturnValue<
                                         FArweaveUploadResult>::success(
                                     Result, QueryResult.meta));
                         })
                         .catch_(Reject)
                         .execute();
                   });
}

inline func::AsyncResult<rtk::QueryReturnValue<FArweaveDownloadResult>>
ExecuteArweaveDownload(const FArweaveDownloadRequest &Request) {
  const FString Url = ArweaveDownloadUrl(Request.Instruction);
  return Url.IsEmpty()
             ? QueryFailure<FArweaveDownloadResult>(
                   TEXT("Missing Arweave download URL"))
             : func::AsyncResult<rtk::QueryReturnValue<
                   FArweaveDownloadResult>>::create(
                   [Request](
                       std::function<void(
                           rtk::QueryReturnValue<FArweaveDownloadResult>)>
                           Resolve,
                       std::function<void(std::string)> Reject) {
                     ExecuteArweaveBaseQuery(ArweaveDownloadFetchArgs(Request))
                         .then([Request, Resolve](
                                   const rtk::QueryReturnValue<FString>
                                       &QueryResult) {
                           Resolve(rtk::QueryReturnValue<
                                   FArweaveDownloadResult>::success(
                               DownloadResultFromQuery(QueryResult,
                                                       Request.Instruction),
                               QueryResult.meta));
                         })
                         .catch_(Reject)
                         .execute();
                   });
}

} // namespace Detail

namespace Endpoints {

inline Thunk<FArweaveUploadResult>
postArweaveUpload(const FArweaveUploadInstruction &Instruction,
                  const FString &SignedPayload, int32 MaxRetries = 3) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Soul"), TEXT("LIST")});
  Detail::FArweaveUploadRequest Request;
  Request.Instruction = Instruction;
  Request.SignedPayload = SignedPayload;
  Request.MaxRetries = MaxRetries;
  return Detail::MakeEndpoint<Detail::FArweaveUploadRequest,
                              FArweaveUploadResult>(
      TEXT("postArweaveUpload"), Request,
      [](const Detail::FArweaveUploadRequest &Arg) {
        return Detail::ExecuteArweaveUploadAttempt(Arg, 0);
      },
      TArray<FApiEndpointTag>(), Invalidates);
}

inline Thunk<FArweaveDownloadResult>
postArweaveDownload(const FArweaveDownloadInstruction &Instruction) {
  TArray<FApiEndpointTag> Invalidates;
  Invalidates.Add(FApiEndpointTag{TEXT("Soul"), TEXT("LIST")});
  Detail::FArweaveDownloadRequest Request;
  Request.Instruction = Instruction;
  return Detail::MakeEndpoint<Detail::FArweaveDownloadRequest,
                              FArweaveDownloadResult>(
      TEXT("postArweaveDownload"), Request, Detail::ExecuteArweaveDownload,
      TArray<FApiEndpointTag>(), Invalidates);
}

} // namespace Endpoints

} // namespace APISlice
