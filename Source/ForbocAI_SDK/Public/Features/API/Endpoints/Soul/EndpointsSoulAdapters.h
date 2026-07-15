#pragma once

#include "Features/API/Endpoints/Rules/RulesAdapters.h"

namespace APISlice::Detail {

/**
 * Encodes the first soul-export request into JSON text.
 * User Story: As soul export flows, I need phase-one request encoding so NPC
 * identity, persona, and state can be sent for upload preparation.
 */
inline FString
EncodeSoulExportPhase1Request(const FSoulExportPhase1Request &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(TEXT("npcIdRef"), Request.NpcIdRef),
          Root->SetStringField(TEXT("persona"), Request.Persona),
          Root->SetObjectField(TEXT("npcState"),
                               JsonInterop::StateToObject(Request.NpcState)),
          ToJsonString(Root));
}

/**
 * Decodes the first soul-export response into upload instructions.
 * User Story: As soul export flows, I need phase-one response decoding so the
 * client can execute upload instructions and preserve signed payload metadata.
 */
inline bool
DecodeSoulExportPhase1Response(const FString &Json,
                               FSoulExportPhase1Response &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
          !Root->HasTypedField<EJson::Object>(TEXT("se1Instruction")))
             ? false
             : (Response.se1Instruction =
                    JsonInterop::UploadInstructionFromObject(
                        Root->GetObjectField(TEXT("se1Instruction"))),
                Response.se1SignedPayload = JsonInterop::JsonStringFromField(
                    Root, TEXT("se1SignedPayload"), TEXT("")),
                Response.se1Signature =
                    JsonInterop::OptionalStringFromField(Root,
                                                         TEXT("se1Signature")),
                true);
}

/**
 * Encodes the soul-export confirmation request into JSON text.
 * User Story: As soul export flows, I need confirmation request encoding so
 * upload results and signatures can be posted back to finalize export.
 */
inline FString
EncodeSoulExportConfirmRequest(const FSoulExportConfirmRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetObjectField(
              TEXT("secUploadResult"),
              JsonInterop::UploadResultToObject(Request.secUploadResult)),
          JsonInterop::SetFieldFromJsonString(Root, TEXT("secSignedPayload"),
                                              Request.secSignedPayload, false),
          Root->SetStringField(TEXT("secSignature"), Request.secSignature),
          ToJsonString(Root));
}

/**
 * Decodes the final soul-export response into typed export details.
 * User Story: As soul export flows, I need final response decoding so the
 * transaction id, Arweave URL, and optional soul payload are available locally.
 */
inline bool DecodeSoulExportResponse(const FString &Json,
                                     FSoulExportResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.TxId = Root->GetStringField(TEXT("txId")),
                Response.ArweaveUrl = Root->GetStringField(TEXT("arweaveUrl")),
                Response.Signature = JsonInterop::OptionalStringFromField(
                    Root, TEXT("signature")),
                Root->HasTypedField<EJson::Object>(TEXT("soul"))
                    ? (Response.Soul = JsonInterop::SoulFromObject(
                           Root->GetObjectField(TEXT("soul"))),
                       void())
                    : void(),
                true);
}

/**
 * Encodes the first soul-import request into JSON text.
 * User Story: As soul import flows, I need phase-one request encoding so the
 * referenced transaction id can be sent to start remote retrieval.
 */
inline FString
EncodeSoulImportPhase1Request(const FSoulImportPhase1Request &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(TEXT("txIdRef"), Request.TxIdRef),
          ToJsonString(Root));
}

/**
 * Decodes the first soul-import response into download instructions.
 * User Story: As soul import flows, I need phase-one response decoding so the
 * client can perform the requested download before confirming import.
 */
inline bool
DecodeSoulImportPhase1Response(const FString &Json,
                               FSoulImportPhase1Response &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
          !Root->HasTypedField<EJson::Object>(TEXT("si1Instruction")))
             ? false
             : (Response.si1Instruction =
                    JsonInterop::DownloadInstructionFromObject(
                        Root->GetObjectField(TEXT("si1Instruction"))),
                true);
}

/**
 * Encodes the soul-import confirmation request into JSON text.
 * User Story: As soul import flows, I need confirmation request encoding so
 * download results can be posted to complete the import handshake.
 */
inline FString
EncodeSoulImportConfirmRequest(const FSoulImportConfirmRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(TEXT("sicTxId"), Request.sicTxId),
          Root->SetObjectField(
              TEXT("sicDownloadResult"),
              JsonInterop::DownloadResultToObject(Request.sicDownloadResult)),
          ToJsonString(Root));
}

/**
 * Decodes an imported NPC payload into a typed NPC value.
 * User Story: As soul import flows, I need imported NPC decoding so runtime
 * systems can consume the restored character data directly.
 */
inline bool DecodeImportedNpc(const FString &Json, FImportedNpc &Npc) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Npc = JsonInterop::ImportedNpcFromObject(Root), true);
}

/**
 * Decodes a soul-verification response into a verification result.
 * User Story: As soul verification flows, I need response decoding so validity
 * and failure reasons map into one runtime verification record.
 */
inline bool DecodeSoulVerifyResponse(const FString &Json,
                                     FSoulVerifyResult &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : [&]() -> bool {
                 const bool bVerifyValid =
                     JsonInterop::detail::TryGetBoolAs(Root,
                                                       TEXT("verifyValid"),
                                                       false);
                 const bool bHasVerifyValid =
                     Root->HasField(TEXT("verifyValid"));
                 Response.bValid =
                     bHasVerifyValid
                         ? bVerifyValid
                         : JsonInterop::detail::TryGetBoolAs(
                               Root, TEXT("valid"), false);

                 const FString VerifyReason =
                     JsonInterop::OptionalStringFromField(Root,
                                                          TEXT("verifyReason"));
                 Response.Reason =
                     !VerifyReason.IsEmpty()
                         ? VerifyReason
                         : JsonInterop::OptionalStringFromField(Root,
                                                                TEXT("reason"));
                 return true;
               }();
}

} // namespace APISlice::Detail
