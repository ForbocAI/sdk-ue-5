#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "Features/Errors/ErrorsAdapters.h"
#include "Features/API/APIApi.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Soul/Transport/TransportAdapters.h"
#include "Features/Config/ConfigAdapters.h"

namespace rtk {

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
                     APISlice::Endpoints::postArweaveDownload(
                         WithApiTransportAuth(Phase1.si1Instruction))(
                         Dispatch, GetState),
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

inline ThunkAction<FSoul, FRuntimeState>
importSoulFromArweaveThunk(const FString &TxId) {
  return importSoulThunk(TxId);
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
                    APISlice::Endpoints::postArweaveDownload(
                        WithApiTransportAuth(Phase1.si1Instruction))(
                        Dispatch, GetState),
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
