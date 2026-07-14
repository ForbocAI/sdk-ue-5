#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "Errors.h"
#include "Features/API/APIApi.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Soul/Transport/TransportAdapters.h"
#include "RuntimeConfig.h"

namespace rtk {

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
                             APISlice::Endpoints::postArweaveUpload(
                                 WithApiTransportAuth(Phase1.se1Instruction),
                                 Phase1.se1SignedPayload)(
                                 Dispatch, GetState),
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
                         APISlice::Endpoints::postArweaveUpload(
                             WithApiTransportAuth(Phase1.se1Instruction),
                             Phase1.se1SignedPayload)(
                             Dispatch, GetState),
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

} // namespace rtk
