#pragma once

#include "Features/Soul/SoulTypes.h"

namespace TypeFactory {

inline FSoul Soul(const FString &Id, const FString &Version,
                  const FString &Name, const FString &Persona,
                  const FAgentState &State, const TArray<FMemoryItem> &Memories,
                  const FString &Signature = TEXT("")) {
  FSoul Value;
  Value.Id = Id;
  Value.Version = Version;
  Value.Name = Name;
  Value.Persona = Persona;
  Value.State = State;
  Value.Memories = Memories;
  Value.Signature = Signature;
  return Value;
}

inline FSoulExportPhase1Request
SoulExportPhase1Request(const FString &NpcIdRef, const FString &Persona,
                        const FAgentState &NpcState) {
  FSoulExportPhase1Request Value;
  Value.NpcIdRef = NpcIdRef;
  Value.Persona = Persona;
  Value.NpcState = NpcState;
  return Value;
}

inline FSoulExportConfirmRequest
SoulExportConfirmRequest(const FArweaveUploadResult &UploadResult,
                         const FString &SignedPayload,
                         const FString &Signature) {
  FSoulExportConfirmRequest Value;
  Value.secUploadResult = UploadResult;
  Value.secSignedPayload = SignedPayload;
  Value.secSignature = Signature;
  return Value;
}

inline FSoulImportPhase1Request
SoulImportPhase1Request(const FString &TxIdRef) {
  FSoulImportPhase1Request Value;
  Value.TxIdRef = TxIdRef;
  return Value;
}

inline FSoulImportConfirmRequest
SoulImportConfirmRequest(const FString &TxId,
                         const FArweaveDownloadResult &DownloadResult) {
  FSoulImportConfirmRequest Value;
  Value.sicTxId = TxId;
  Value.sicDownloadResult = DownloadResult;
  return Value;
}

inline FSoulExportResult SoulExportResult(const FString &TxId,
                                          const FString &Url,
                                          const FSoul &SoulValue) {
  FSoulExportResult Value;
  Value.TxId = TxId;
  Value.Url = Url;
  Value.Soul = SoulValue;
  return Value;
}

inline FSoulVerifyResult SoulVerifyResult(bool bValid,
                                          const FString &Reason = TEXT("")) {
  FSoulVerifyResult Value;
  Value.bValid = bValid;
  Value.Reason = Reason;
  return Value;
}

} // namespace TypeFactory
