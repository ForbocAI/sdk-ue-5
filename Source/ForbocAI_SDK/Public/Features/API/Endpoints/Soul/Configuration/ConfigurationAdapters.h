#pragma once

#include "Features/API/Endpoints/Soul/Configuration/ConfigurationTypes.h"
#include "Features/Data/DataAdapters.h"

namespace APISlice::Endpoints::SoulConfiguration {

/**
 * User Story: As a stateless Soul endpoint, I need every metadata field loaded
 * from the canonical API contract so UE and TS send identical payloads.
 * @fn inline FSoulEndpointConfigurationData readSoulEndpointConfigurationData()
 */
inline FSoulEndpointConfigurationData readSoulEndpointConfigurationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/soul.json"));
  const TSharedRef<FJsonObject> PreparationRequest =
      DataAdapters::ReadObjectField(Source, TEXT("exportPreparationRequest"));
  const TSharedRef<FJsonObject> PreparationResponse =
      DataAdapters::ReadObjectField(Source, TEXT("exportPreparationResponse"));
  const TSharedRef<FJsonObject> ConfirmationRequest =
      DataAdapters::ReadObjectField(Source,
                                    TEXT("exportConfirmationRequest"));
  const TSharedRef<FJsonObject> ExportResponse =
      DataAdapters::ReadObjectField(Source, TEXT("exportResponse"));
  const TSharedRef<FJsonObject> VerificationRequest =
      DataAdapters::ReadObjectField(Source, TEXT("verificationRequest"));
  const TSharedRef<FJsonObject> VerifyResponse =
      DataAdapters::ReadObjectField(Source, TEXT("verifyResponse"));
  return {
      {DataAdapters::ReadStringField(PreparationRequest, TEXT("npcId")),
       DataAdapters::ReadStringField(PreparationRequest,
                                     TEXT("transactionId")),
       DataAdapters::ReadStringField(PreparationRequest,
                                     TEXT("payloadDigest")),
       DataAdapters::ReadStringField(PreparationRequest, TEXT("provider"))},
      {{DataAdapters::ReadStringField(PreparationResponse, TEXT("npcId")),
        DataAdapters::ReadStringField(PreparationResponse,
                                      TEXT("transactionId")),
        DataAdapters::ReadStringField(PreparationResponse,
                                      TEXT("payloadDigest")),
        DataAdapters::ReadStringField(PreparationResponse, TEXT("provider"))},
       DataAdapters::ReadStringField(PreparationResponse,
                                     TEXT("confirmationToken"))},
      {DataAdapters::ReadStringField(ConfirmationRequest, TEXT("storageUrl")),
       DataAdapters::ReadStringField(ConfirmationRequest,
                                     TEXT("providerStatus"))},
      {DataAdapters::ReadStringField(ExportResponse, TEXT("transactionId")),
       DataAdapters::ReadStringField(ExportResponse, TEXT("storageUrl")),
       DataAdapters::ReadStringField(ExportResponse, TEXT("signature"))},
      {DataAdapters::ReadStringField(VerificationRequest, TEXT("npcId")),
       DataAdapters::ReadStringField(VerificationRequest,
                                     TEXT("payloadDigest")),
       DataAdapters::ReadStringField(VerificationRequest, TEXT("provider")),
       DataAdapters::ReadStringField(VerificationRequest, TEXT("signature"))},
      {DataAdapters::ReadStringField(VerifyResponse, TEXT("valid")),
       DataAdapters::ReadStringField(VerifyResponse, TEXT("reason"))}};
}

/**
 * User Story: As every Soul API endpoint, I need one immutable metadata schema
 * per process so plaintext character data can never enter the API payload.
 * @fn inline const FSoulEndpointConfigurationData &soulEndpointData()
 */
inline const FSoulEndpointConfigurationData &soulEndpointData() {
  static const FSoulEndpointConfigurationData Data =
      readSoulEndpointConfigurationData();
  return Data;
}

} // namespace APISlice::Endpoints::SoulConfiguration
