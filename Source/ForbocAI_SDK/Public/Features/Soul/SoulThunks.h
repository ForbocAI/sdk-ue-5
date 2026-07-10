#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

#include "Async/Async.h"
#include "Core/JsonInterop.h"
#include "Features/API/APISlice.h"
#include "Errors.h"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCSlice.h"
#include "RuntimeConfig.h"
#include "Features/Soul/SoulSlice.h"
#include "HAL/PlatformProcess.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

namespace rtk {

inline FString ApiTransportAuthHeader() {
  const FString ApiKey = SDKConfig::GetApiKey();
  return ApiKey.IsEmpty() ? FString() : FString(TEXT("Bearer ")) + ApiKey;
}

inline bool IsApiOwnedTransportUrl(const FString &Url) {
  const FString ApiUrl = SDKConfig::GetApiUrl();
  return !Url.IsEmpty() && !ApiUrl.IsEmpty() && Url.StartsWith(ApiUrl);
}

inline FArweaveUploadInstruction
WithApiTransportAuth(FArweaveUploadInstruction Instruction) {
  const FString Url = !Instruction.UploadUrl.IsEmpty()
                          ? Instruction.UploadUrl
                          : Instruction.GatewayUrl;
  const FString Header = ApiTransportAuthHeader();
  return IsApiOwnedTransportUrl(Url) && !Header.IsEmpty()
             ? (Instruction.AuiAuthHeader = Header, Instruction)
             : Instruction;
}

inline FArweaveDownloadInstruction
WithApiTransportAuth(FArweaveDownloadInstruction Instruction) {
  const FString Url = !Instruction.DownloadUrl.IsEmpty()
                          ? Instruction.DownloadUrl
                          : Instruction.GatewayUrl;
  const FString Header = ApiTransportAuthHeader();
  return IsApiOwnedTransportUrl(Url) && !Header.IsEmpty()
             ? (Instruction.AdiAuthHeader = Header, Instruction)
             : Instruction;
}

namespace detail {

/**
 * Upload a signed soul payload to API-owned soul storage.
 * User Story: As soul export flows, I need uploads handled with consistent
 * retry and result semantics so remote persistence behaves predictably.
 *
 * Matches TS SDK handler_ArweaveUpload behaviour:
 *  - 3 attempts max (configurable via MaxRetries)
 *  - Exponential backoff between retries: 250ms * 2^(attempt-1)
 *  - 60-second HTTP timeout per attempt
 *  - txId derivation order:
 *      1. (UE-only enhancement) x-id response header
 *      2. Response JSON ".id" field        (same as TS)
 *  - On exhausted retries, resolves with error result (does NOT reject)
 */
inline func::AsyncResult<FArweaveUploadResult>
UploadSignedSoul(const FArweaveUploadInstruction &Instruction,
                 const FString &SignedPayload,
                 int32 MaxRetries = 3) {
  return func::AsyncResult<FArweaveUploadResult>::create(
      [Instruction, SignedPayload, MaxRetries](
          std::function<void(FArweaveUploadResult)> Resolve,
          std::function<void(std::string)> Reject) {
        const FString Url = !Instruction.UploadUrl.IsEmpty()
                                ? Instruction.UploadUrl
                                : Instruction.GatewayUrl;
        const FString Payload =
            !Instruction.PayloadJson.IsEmpty() ? Instruction.PayloadJson
                                               : SignedPayload;
        Url.IsEmpty()
            ? (Reject("Missing Arweave upload URL"), void())
            : [&]() {
                struct FRetryState {
                  int32 Attempt = 0;
                  int32 MaxRetries = 3;
                  FString Url;
                  FString Payload;
                  FArweaveUploadInstruction Instruction;
                  std::function<void(FArweaveUploadResult)> Resolve;
                  std::function<void(std::string)> Reject;
                };

                TSharedPtr<FRetryState> State = MakeShared<FRetryState>();
                State->MaxRetries = MaxRetries;
                State->Url = Url;
                State->Payload = Payload;
                State->Instruction = Instruction;
                State->Resolve = Resolve;
                State->Reject = Reject;

                TSharedPtr<TFunction<void()>> TryOnce =
                    MakeShared<TFunction<void()>>();
                *TryOnce = [State, TryOnce]() {
                  State->Attempt += 1;

                  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
                      FHttpModule::Get().CreateRequest();
                  Request->SetURL(State->Url);
                  Request->SetVerb(TEXT("POST"));
                  Request->SetTimeout(60.0f);
                  Request->SetHeader(
                      TEXT("Content-Type"),
                      State->Instruction.ContentType.IsEmpty()
                          ? TEXT("application/octet-stream")
                          : State->Instruction.ContentType);
                  !State->Instruction.AuiAuthHeader.IsEmpty()
                      ? (Request->SetHeader(
                             TEXT("Authorization"),
                             State->Instruction.AuiAuthHeader),
                         void())
                      : void();
                  !State->Instruction.TagsJson.IsEmpty()
                      ? (Request->SetHeader(TEXT("X-Forboc-Tags"),
                                            State->Instruction.TagsJson),
                         void())
                      : void();
                  Request->SetContentAsString(State->Payload);

                  Request->OnProcessRequestComplete().BindLambda(
                      [State, TryOnce](FHttpRequestPtr Req,
                                       FHttpResponsePtr Res,
                                       bool bWasSuccessful) {
                        const auto ScheduleRetry = [&State, &TryOnce]() {
                          const float BackoffSec =
                              0.250f *
                              FMath::Pow(
                                  2.0f,
                                  static_cast<float>(State->Attempt - 1));
                          AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
                                    [State, TryOnce, BackoffSec]() {
                                      FPlatformProcess::Sleep(BackoffSec);
                                      AsyncTask(ENamedThreads::GameThread,
                                                [TryOnce]() { (*TryOnce)(); });
                                    });
                        };

                        (!bWasSuccessful || !Res.IsValid())
                            ? (State->Attempt < State->MaxRetries
                                   ? (ScheduleRetry(), void())
                                   : [&State]() {
                                       FArweaveUploadResult Result;
                                       Result.StatusCode = 0;
                                       Result.bSuccess = false;
                                       Result.Status = TEXT("0");
                                       Result.Error = TEXT(
                                           "upload_request_failed:network_error");
                                       State->Resolve(Result);
                                     }())
                            : [&State, &Res, &ScheduleRetry]() {
                                FArweaveUploadResult Result;
                                Result.ResponseJson = Res->GetContentAsString();
                                Result.StatusCode = Res->GetResponseCode();
                                Result.Status =
                                    FString::FromInt(Result.StatusCode);
                                const bool bHttpSuccess =
                                    Result.StatusCode >= 200 &&
                                    Result.StatusCode < 300;

                                (!bHttpSuccess &&
                                 State->Attempt < State->MaxRetries)
                                    ? (ScheduleRetry(), void())
                                    : (Result.TxId =
                                           Res->GetHeader(TEXT("x-id")),
                                       Result.TxId.IsEmpty()
                                           ? [&Result]() {
                                               TSharedPtr<FJsonObject>
                                                   ResponseObject;
                                               (JsonInterop::ParseJsonObject(
                                                    Result.ResponseJson,
                                                    ResponseObject) &&
                                                ResponseObject.IsValid())
                                                   ? (Result.TxId =
                                                          ResponseObject
                                                              ->GetStringField(
                                                                  TEXT("id")),
                                                      void())
                                                   : void();
                                             }()
                                           : void(),
                                       Result.bSuccess =
                                           bHttpSuccess &&
                                           !Result.TxId.IsEmpty(),
                                       Result.Error =
                                           Result.bSuccess
                                               ? FString(TEXT(""))
                                               : (bHttpSuccess
                                                      ? FString(TEXT(
                                                            "upload_missing_tx_id"))
                                                      : FString::Printf(
                                                            TEXT("upload_failed_status_%d"),
                                                            Result.StatusCode)),
                                       Result.ArweaveUrl =
                                           !State->Instruction.GatewayUrl
                                                .IsEmpty()
                                               ? State->Instruction.GatewayUrl +
                                                     TEXT("/") + Result.TxId
                                               : TEXT(""),
                                       State->Resolve(Result), void());
                              }();
                      });
                  Request->ProcessRequest();
                };

                (*TryOnce)();
              }();
      });
}

/**
 * Download a soul payload from API-owned soul storage.
 * User Story: As soul import flows, I need download results normalized so
 * callers can validate and import remote payloads consistently.
 */
inline func::AsyncResult<FArweaveDownloadResult>
DownloadSoulPayload(const FArweaveDownloadInstruction &Instruction) {
  return func::AsyncResult<FArweaveDownloadResult>::create(
      [Instruction](std::function<void(FArweaveDownloadResult)> Resolve,
                    std::function<void(std::string)> Reject) {
        const FString Url =
            !Instruction.DownloadUrl.IsEmpty() ? Instruction.DownloadUrl
                                               : Instruction.GatewayUrl;

        Url.IsEmpty()
            ? (Reject("Missing Arweave download URL"), void())
            : [&]() {
                TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
                    FHttpModule::Get().CreateRequest();
                Request->SetURL(Url);
                Request->SetVerb(TEXT("GET"));
                Request->SetTimeout(60.0f);
                !Instruction.AdiAuthHeader.IsEmpty()
                    ? (Request->SetHeader(TEXT("Authorization"),
                                          Instruction.AdiAuthHeader),
                       void())
                    : void();
                Request->OnProcessRequestComplete().BindLambda(
                    [Instruction, Resolve, Reject](FHttpRequestPtr Req,
                                                   FHttpResponsePtr Res,
                                                   bool bWasSuccessful) {
                      (!bWasSuccessful || !Res.IsValid())
                          ? (Reject("Arweave download failed"), void())
                          : [&]() {
                              FArweaveDownloadResult Result;
                              Result.TxId =
                                  !Instruction.ExpectedTxId.IsEmpty()
                                      ? Instruction.ExpectedTxId
                                      : Instruction.TxId;
                              Result.Payload = Res->GetContentAsString();
                              Result.BodyJson = Result.Payload;
                              Result.StatusCode = Res->GetResponseCode();
                              Result.bSuccess = Result.StatusCode >= 200 &&
                                                Result.StatusCode < 300;
                              Result.Status =
                                  FString::FromInt(Result.StatusCode);
                              Result.Error =
                                  Result.bSuccess
                                      ? FString(TEXT(""))
                                      : FString::Printf(
                                            TEXT("download_failed_status_%d"),
                                            Result.StatusCode);
                              Result.ResponseJson = Result.Payload;
                              Resolve(Result);
                            }();
                    });
                Request->ProcessRequest();
              }();
      });
}

} // namespace detail

/**
 * Soul thunks (mirrors TS soulSlice.ts)
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

inline ThunkAction<FSoulExportResult, FRuntimeState>
exportSoulThunk(const FString &NpcId) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoulExportResult> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FSoulExportResult>(ApiKeyError.value)
        : [&]() -> func::AsyncResult<FSoulExportResult> {
            const auto Npc = NPCSlice::selectNPCById(GetState().NPCs, NpcId);
            return !Npc.hasValue
                ? detail::RejectAsync<FSoulExportResult>(
                      TEXT("NPC not found"))
                : (Dispatch(SoulSlice::Actions::remoteExportSoulPending()),
                   func::AsyncChain::then<FSoulExportPhase1Response,
                                          FSoulExportResult>(
                       APISlice::Endpoints::postSoulExport(
                           NpcId,
                           TypeFactory::SoulExportPhase1Request(
                               NpcId, Npc.value.Persona, Npc.value.State))(
                           Dispatch, GetState),
                       [NpcId, Dispatch,
                        GetState](const FSoulExportPhase1Response &Phase1) {
                         return func::AsyncChain::then<FArweaveUploadResult,
                                                       FSoulExportResult>(
                             detail::UploadSignedSoul(
                                 WithApiTransportAuth(Phase1.se1Instruction),
                                 Phase1.se1SignedPayload),
                             [NpcId, Phase1, Dispatch, GetState](
                                 const FArweaveUploadResult &UploadResult) {
                               const FSoulExportConfirmRequest Confirm =
                                   TypeFactory::SoulExportConfirmRequest(
                                       UploadResult,
                                       Phase1.se1SignedPayload,
                                       Phase1.se1Signature);

                               return func::AsyncChain::then<
                                   FSoulExportResponse, FSoulExportResult>(
                                   APISlice::Endpoints::
                                       postSoulExportConfirm(NpcId, Confirm)(
                                           Dispatch, GetState),
                                   [Dispatch](const FSoulExportResponse
                                                  &Response) {
                                     const FSoulExportResult Result =
                                         TypeFactory::SoulExportResult(
                                             Response.TxId,
                                             Response.ArweaveUrl,
                                             Response.Soul);
                                     Dispatch(
                                         SoulSlice::Actions::
                                             remoteExportSoulSuccess(Result));
                                     return detail::ResolveAsync(Result);
                                   });
                             });
                       })
                       .catch_([Dispatch](std::string Error) {
                         Dispatch(SoulSlice::Actions::remoteExportSoulFailed(
                             Errors::extractThunkErrorMessage(Error)));
                       }));
          }();
  };
}

inline ThunkAction<FSoulExportResult, FRuntimeState>
exportSoulThunk(const FSoul &Soul) {
  return [Soul](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoulExportResult> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FSoulExportResult>(ApiKeyError.value)
        : Soul.Id.IsEmpty()
            ? detail::RejectAsync<FSoulExportResult>(
                  TEXT("Soul ID is required"))
            : (Dispatch(SoulSlice::Actions::remoteExportSoulPending()),
               func::AsyncChain::then<FSoulExportPhase1Response,
                                      FSoulExportResult>(
                   APISlice::Endpoints::postSoulExport(
                       Soul.Id,
                       TypeFactory::SoulExportPhase1Request(
                           Soul.Id, Soul.Persona, Soul.State))(Dispatch,
                                                                GetState),
                   [Soul, Dispatch,
                    GetState](const FSoulExportPhase1Response &Phase1) {
                     return func::AsyncChain::then<FArweaveUploadResult,
                                                   FSoulExportResult>(
                         detail::UploadSignedSoul(
                             WithApiTransportAuth(Phase1.se1Instruction),
                             Phase1.se1SignedPayload),
                         [Soul, Phase1, Dispatch, GetState](
                             const FArweaveUploadResult &UploadResult) {
                           const FSoulExportConfirmRequest Confirm =
                               TypeFactory::SoulExportConfirmRequest(
                                   UploadResult, Phase1.se1SignedPayload,
                                   Phase1.se1Signature);

                           return func::AsyncChain::then<
                               FSoulExportResponse, FSoulExportResult>(
                               APISlice::Endpoints::postSoulExportConfirm(
                                   Soul.Id, Confirm)(Dispatch, GetState),
                               [Soul,
                                Dispatch](const FSoulExportResponse
                                              &Response) {
                                 const FSoulExportResult Result =
                                     TypeFactory::SoulExportResult(
                                         Response.TxId, Response.ArweaveUrl,
                                         Response.Soul.Id.IsEmpty()
                                             ? Soul
                                             : Response.Soul);
                                 Dispatch(
                                     SoulSlice::Actions::
                                         remoteExportSoulSuccess(Result));
                                 return detail::ResolveAsync(Result);
                               });
                         });
                   })
                   .catch_([Dispatch](std::string Error) {
                     Dispatch(SoulSlice::Actions::remoteExportSoulFailed(
                         Errors::extractThunkErrorMessage(Error)));
                   }));
  };
}

inline ThunkAction<FSoul, FRuntimeState>
importSoulThunk(const FString &TxId) {
  return [TxId](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoul> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FSoul>(ApiKeyError.value)
        : (Dispatch(SoulSlice::Actions::importSoulPending()),
           func::AsyncChain::then<FSoulImportPhase1Response, FSoul>(
               APISlice::Endpoints::postNpcImport(
                   TypeFactory::SoulImportPhase1Request(TxId))(Dispatch,
                                                               GetState),
               [TxId, Dispatch,
                GetState](const FSoulImportPhase1Response &Phase1) {
                 return func::AsyncChain::then<FArweaveDownloadResult, FSoul>(
                     detail::DownloadSoulPayload(
                         WithApiTransportAuth(Phase1.si1Instruction)),
                     [TxId, Dispatch, GetState](
                         const FArweaveDownloadResult &DownloadResult) {
                       return func::AsyncChain::then<FImportedNpc, FSoul>(
                           APISlice::Endpoints::postNpcImportConfirm(
                               TypeFactory::SoulImportConfirmRequest(
                                   TxId, DownloadResult))(Dispatch, GetState),
                           [Dispatch,
                            TxId](const FImportedNpc &ImportedNpc) {
                             FSoul Soul = TypeFactory::Soul(
                                 ImportedNpc.NpcId.IsEmpty()
                                     ? TxId
                                     : ImportedNpc.NpcId,
                                 TEXT("2.0.0"), ImportedNpc.NpcId,
                                 ImportedNpc.Persona,
                                 TypeFactory::AgentState(
                                     ImportedNpc.DataJson),
                                 TArray<FMemoryItem>());
                             Dispatch(SoulSlice::Actions::importSoulSuccess(
                                 Soul));
                             return detail::ResolveAsync(Soul);
                           });
                     });
               })
               .catch_([Dispatch](std::string Error) {
                 Dispatch(SoulSlice::Actions::importSoulFailed(
                     Errors::extractThunkErrorMessage(Error)));
               }));
  };
}

/**
 * Soul convenience thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

inline ThunkAction<FSoul, FRuntimeState>
localExportSoulThunk(const FString &NpcId = TEXT("")) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoul> {
    const FString TargetNpcId =
        NpcId.IsEmpty() ? NPCSlice::selectActiveNpcId(GetState().NPCs) : NpcId;
    const auto Npc = NPCSlice::selectNPCById(GetState().NPCs, TargetNpcId);
    return !Npc.hasValue
        ? detail::RejectAsync<FSoul>(TEXT("NPC not found"))
        : detail::ResolveAsync(TypeFactory::Soul(
              TargetNpcId, TEXT("1.0.0"), TEXT("NPC"), Npc.value.Persona,
              Npc.value.State,
              MemorySlice::selectAllMemories(GetState().Memory)));
  };
}

/**
 * Imports a Soul from a local JSON representation (no network).
 * Sets NPC info from the soul data and dispatches importSoulSuccess.
 * Mirrors TS localImportSoulThunk in soulSlice.ts.
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 */
inline ThunkAction<FSoul, FRuntimeState>
localImportSoulThunk(const FSoul &Soul) {
  return [Soul](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoul> {
    return Soul.Id.IsEmpty()
        ? detail::RejectAsync<FSoul>(TEXT("Soul ID is required"))
        : [&]() -> func::AsyncResult<FSoul> {
            FNPCInternalState Npc;
            Npc.Id = Soul.Id;
            Npc.Persona = Soul.Persona;
            Npc.State = Soul.State;
            Dispatch(NPCSlice::Actions::setNPCInfo(Npc));
            Dispatch(SoulSlice::Actions::importSoulSuccess(Soul));
            return detail::ResolveAsync(Soul);
          }();
  };
}

inline ThunkAction<FSoulExportResult, FRuntimeState>
remoteExportSoulThunk(const FString &NpcId = TEXT("")) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoulExportResult> {
    const FString TargetNpcId =
        NpcId.IsEmpty() ? NPCSlice::selectActiveNpcId(GetState().NPCs) : NpcId;
    return exportSoulThunk(TargetNpcId)(Dispatch, GetState);
  };
}

inline ThunkAction<FSoul, FRuntimeState>
importSoulFromArweaveThunk(const FString &TxId) {
  return importSoulThunk(TxId);
}

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

inline ThunkAction<FImportedNpc, FRuntimeState>
importNpcFromSoulThunk(const FString &TxId) {
  return [TxId](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FImportedNpc> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FImportedNpc>(ApiKeyError.value)
        : func::AsyncChain::then<FSoulImportPhase1Response, FImportedNpc>(
              APISlice::Endpoints::postNpcImport(
                  TypeFactory::SoulImportPhase1Request(TxId))(Dispatch,
                                                              GetState),
              [TxId, Dispatch,
               GetState](const FSoulImportPhase1Response &Phase1) {
                return func::AsyncChain::then<FArweaveDownloadResult,
                                              FImportedNpc>(
                    detail::DownloadSoulPayload(
                        WithApiTransportAuth(Phase1.si1Instruction)),
                    [TxId, Dispatch, GetState](
                        const FArweaveDownloadResult &DownloadResult) {
                      return func::AsyncChain::then<FImportedNpc,
                                                    FImportedNpc>(
                          APISlice::Endpoints::postNpcImportConfirm(
                              TypeFactory::SoulImportConfirmRequest(
                                  TxId, DownloadResult))(Dispatch, GetState),
                          [Dispatch,
                           TxId](const FImportedNpc &ImportedNpc) {
                            FNPCInternalState Npc;
                            Npc.Id = ImportedNpc.NpcId.IsEmpty()
                                         ? TxId
                                         : ImportedNpc.NpcId;
                            Npc.Persona = ImportedNpc.Persona;
                            Npc.State = TypeFactory::AgentState(
                                ImportedNpc.DataJson);
                            Dispatch(NPCSlice::Actions::setNPCInfo(Npc));
                            return detail::ResolveAsync(ImportedNpc);
                          });
                    });
              });
  };
}

} // namespace rtk
