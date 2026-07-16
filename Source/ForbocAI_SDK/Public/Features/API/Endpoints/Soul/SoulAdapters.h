#pragma once

#include "Core/JsonInterop.h"
#include "Features/API/Endpoints/Soul/Configuration/ConfigurationAdapters.h"
#include "Features/Soul/SoulTypes.h"

namespace APISlice::Detail {

namespace SoulConfiguration = APISlice::Endpoints::SoulConfiguration;

/**
 * User Story: As a Soul exporter, I need only immutable encrypted-payload
 * metadata encoded for API authorization.
 * @fn inline FString EncodeSoulExportPreparationRequest( const FSoulExportPreparationRequest &Request)
 */
inline FString EncodeSoulExportPreparationRequest(
    const FSoulExportPreparationRequest &Request) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(Data.PreparationRequest.NpcId,
                               Request.NpcIdRef),
          Root->SetStringField(Data.PreparationRequest.TransactionId,
                               Request.TransactionId),
          Root->SetStringField(Data.PreparationRequest.PayloadDigest,
                               Request.PayloadDigest),
          Root->SetStringField(Data.PreparationRequest.Provider,
                               Request.Provider),
          JsonInterop::StringifyObject(Root));
}

/**
 * User Story: As a Soul exporter, I need every API authorization field
 * validated before provider upload begins.
 * @fn inline bool DecodeSoulExportPreparationResponse( const FString &Json, FSoulExportPreparation &Response)
 */
inline bool DecodeSoulExportPreparationResponse(
    const FString &Json, FSoulExportPreparation &Response) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  TSharedPtr<FJsonObject> Root;
  const bool bValid = JsonInterop::ParseJsonObject(Json, Root) &&
      Root.IsValid() &&
      Root->HasTypedField<EJson::String>(Data.PreparationResponse.NpcId) &&
      Root->HasTypedField<EJson::String>(
          Data.PreparationResponse.TransactionId) &&
      Root->HasTypedField<EJson::String>(
          Data.PreparationResponse.PayloadDigest) &&
      Root->HasTypedField<EJson::String>(Data.PreparationResponse.Provider) &&
      Root->HasTypedField<EJson::String>(
          Data.PreparationResponse.ConfirmationToken);
  return !bValid
             ? false
             : (Response.NpcIdRef = Root->GetStringField(
                    Data.PreparationResponse.NpcId),
                Response.TransactionId = Root->GetStringField(
                    Data.PreparationResponse.TransactionId),
                Response.PayloadDigest = Root->GetStringField(
                    Data.PreparationResponse.PayloadDigest),
                Response.Provider = Root->GetStringField(
                    Data.PreparationResponse.Provider),
                Response.ConfirmationToken = Root->GetStringField(
                    Data.PreparationResponse.ConfirmationToken),
                true);
}

/**
 * User Story: As a Soul exporter, I need provider evidence combined with the
 * exact API preparation token without sending Soul plaintext.
 * @fn inline FString EncodeSoulExportConfirmationRequest( const FSoulExportConfirmationRequest &Request)
 */
inline FString EncodeSoulExportConfirmationRequest(
    const FSoulExportConfirmationRequest &Request) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(Data.PreparationResponse.NpcId,
                               Request.NpcIdRef),
          Root->SetStringField(Data.PreparationResponse.TransactionId,
                               Request.TransactionId),
          Root->SetStringField(Data.PreparationResponse.PayloadDigest,
                               Request.PayloadDigest),
          Root->SetStringField(Data.PreparationResponse.Provider,
                               Request.Provider),
          Root->SetStringField(Data.PreparationResponse.ConfirmationToken,
                               Request.ConfirmationToken),
          Root->SetStringField(Data.ConfirmationRequest.StorageUrl,
                               Request.StorageUrl),
          Root->SetNumberField(Data.ConfirmationRequest.ProviderStatus,
                               Request.ProviderStatus),
          JsonInterop::StringifyObject(Root));
}

/**
 * User Story: As a Soul catalog owner, I need the API confirmation normalized
 * before its signature can authorize durable local metadata.
 * @fn inline bool DecodeSoulExportConfirmationResponse( const FString &Json, FSoulExportResponse &Response)
 */
inline bool DecodeSoulExportConfirmationResponse(
    const FString &Json, FSoulExportResponse &Response) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  TSharedPtr<FJsonObject> Root;
  const bool bValid = JsonInterop::ParseJsonObject(Json, Root) &&
      Root.IsValid() &&
      Root->HasTypedField<EJson::String>(Data.ExportResponse.TransactionId) &&
      Root->HasTypedField<EJson::String>(Data.ExportResponse.StorageUrl) &&
      Root->HasTypedField<EJson::String>(Data.ExportResponse.Signature);
  return !bValid
             ? false
             : (Response.TxId = Root->GetStringField(
                    Data.ExportResponse.TransactionId),
                Response.StorageUrl = Root->GetStringField(
                    Data.ExportResponse.StorageUrl),
                Response.Signature = Root->GetStringField(
                    Data.ExportResponse.Signature),
                true);
}

/**
 * User Story: As a Soul verifier, I need only catalog authorization metadata
 * encoded so decrypted character content remains SDK-owned.
 * @fn inline FString EncodeSoulVerificationRequest( const FSoulVerificationRequest &Request)
 */
inline FString EncodeSoulVerificationRequest(
    const FSoulVerificationRequest &Request) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(Data.VerificationRequest.NpcId,
                               Request.NpcIdRef),
          Root->SetStringField(Data.VerificationRequest.PayloadDigest,
                               Request.PayloadDigest),
          Root->SetStringField(Data.VerificationRequest.Provider,
                               Request.Provider),
          Root->SetStringField(Data.VerificationRequest.Signature,
                               Request.Signature),
          JsonInterop::StringifyObject(Root));
}

/**
 * User Story: As a Soul verifier, I need canonical verification fields checked
 * before trust state enters Redux.
 * @fn inline bool DecodeSoulVerifyResponse(const FString &Json, FSoulVerifyResult &Response)
 */
inline bool DecodeSoulVerifyResponse(const FString &Json,
                                     FSoulVerifyResult &Response) {
  const auto &Data = SoulConfiguration::soulEndpointData();
  TSharedPtr<FJsonObject> Root;
  bool bValid = false;
  return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
                 !Root->TryGetBoolField(Data.VerifyResponse.Valid, bValid) ||
                 !JsonInterop::HasOptionalFieldType(
                     Root, Data.VerifyResponse.Reason, EJson::String)
             ? false
             : (Response.bValid = bValid,
                Response.Reason = JsonInterop::OptionalStringFromField(
                    Root, Data.VerifyResponse.Reason),
                true);
}

} // namespace APISlice::Detail
