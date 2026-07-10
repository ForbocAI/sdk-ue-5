#pragma once

#include "Features/API/Endpoints/EndpointsTypes.h"

namespace APISlice {
namespace Endpoints {

inline Thunk<FSoulExportPhase1Response>
postSoulExport(const FString &NpcId,
               const FSoulExportPhase1Request &Request) {
  return Detail::MakePostWithCodec<FSoulExportPhase1Request,
                                   FSoulExportPhase1Response>(
      TEXT("postSoulExport"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/soul/export"),
      Request, Detail::EncodeSoulExportPhase1Request,
      Detail::DecodeSoulExportPhase1Response);
}

inline Thunk<FSoulExportResponse>
postSoulExportConfirm(const FString &NpcId,
                      const FSoulExportConfirmRequest &Request) {
  return Detail::MakePostWithCodec<FSoulExportConfirmRequest,
                                   FSoulExportResponse>(
      TEXT("postSoulExportConfirm"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/") + Detail::Encode(NpcId) +
          TEXT("/soul/confirm"),
      Request, Detail::EncodeSoulExportConfirmRequest,
      Detail::DecodeSoulExportResponse);
}

inline Thunk<FSoulListResponse> getSouls(int32 Limit = 50) {
  return Detail::MakeGet<FSoulListResponse>(
      TEXT("getSouls"),
      SDKConfig::GetApiUrl() + TEXT("/souls?limit=") + FString::FromInt(Limit));
}

inline Thunk<FSoulImportPhase1Response> getSoulImport(const FString &TxId) {
  return Detail::MakeGet<FSoulImportPhase1Response>(
      TEXT("getSoulImport"),
      SDKConfig::GetApiUrl() + TEXT("/souls/") + Detail::Encode(TxId));
}

inline Thunk<FSoulVerifyResult> postSoulVerify(const FString &TxId) {
  return Detail::MakePostRawWithCodec<FSoulVerifyResult>(
      TEXT("postSoulVerify"),
      SDKConfig::GetApiUrl() + TEXT("/souls/") + Detail::Encode(TxId) +
          TEXT("/verify"),
      TEXT("{}"), Detail::DecodeSoulVerifyResponse);
}

inline Thunk<FSoulImportPhase1Response>
postNpcImport(const FSoulImportPhase1Request &Request) {
  return Detail::MakePostWithCodec<FSoulImportPhase1Request,
                                   FSoulImportPhase1Response>(
      TEXT("postNpcImport"), SDKConfig::GetApiUrl() + TEXT("/npcs/import"),
      Request, Detail::EncodeSoulImportPhase1Request,
      Detail::DecodeSoulImportPhase1Response);
}

inline Thunk<FImportedNpc>
postNpcImportConfirm(const FSoulImportConfirmRequest &Request) {
  return Detail::MakePostWithCodec<FSoulImportConfirmRequest, FImportedNpc>(
      TEXT("postNpcImportConfirm"),
      SDKConfig::GetApiUrl() + TEXT("/npcs/import/confirm"), Request,
      Detail::EncodeSoulImportConfirmRequest, Detail::DecodeImportedNpc);
}

} // namespace Endpoints
} // namespace APISlice
