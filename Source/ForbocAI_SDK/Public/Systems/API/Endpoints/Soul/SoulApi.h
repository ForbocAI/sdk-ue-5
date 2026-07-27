#pragma once

#include "Systems/API/APIAdapters.h"
#include "Components/API/Endpoints/EndpointsTypes.h"

namespace APISlice::Endpoints {

/**
 * User Story: As a Soul exporter, I need the API to bind the precomputed
 * immutable transaction ID to the encrypted payload digest before upload.
 * @fn inline Thunk<FSoulExportPreparation> postSoulExportPreparation( const FString &NpcId, const FSoulExportPreparationRequest &Request)
 */
inline Thunk<FSoulExportPreparation> postSoulExportPreparation(
    const FString &NpcId, const FSoulExportPreparationRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakePostWithCodec<FSoulExportPreparationRequest,
                                   FSoulExportPreparation>(
      Data.Names.PostSoulExportPreparation,
      Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Soul,
           Data.Segments.Export}),
      Request, Detail::EncodeSoulExportPreparationRequest,
      Detail::DecodeSoulExportPreparationResponse,
      {soulTagAdapter(NpcId), soulTagAdapter(Request.TransactionId)});
}

/**
 * User Story: As a Soul exporter, I need provider evidence confirmed against
 * the API-issued token before durable local catalog publication.
 * @fn inline Thunk<FSoulExportResponse> postSoulExportConfirmation( const FString &NpcId, const FSoulExportConfirmationRequest &Request)
 */
inline Thunk<FSoulExportResponse> postSoulExportConfirmation(
    const FString &NpcId, const FSoulExportConfirmationRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakePostWithCodec<FSoulExportConfirmationRequest,
                                   FSoulExportResponse>(
      Data.Names.PostSoulExportConfirmation,
      Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Soul,
           Data.Segments.Confirm}),
      Request, Detail::EncodeSoulExportConfirmationRequest,
      Detail::DecodeSoulExportConfirmationResponse,
      {soulTagAdapter(Request.TransactionId), soulListTagAdapter()});
}

/**
 * User Story: As a Soul importer, I need catalog metadata verified without
 * sending decrypted character content to the API.
 * @fn inline Thunk<FSoulVerifyResult> postSoulVerification(const FString &TxId, const FSoulVerificationRequest &Request)
 */
inline Thunk<FSoulVerifyResult>
postSoulVerification(const FString &TxId,
                     const FSoulVerificationRequest &Request) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakePostQueryWithCodec<FSoulVerificationRequest,
                                        FSoulVerifyResult>(
      Data.Names.PostSoulVerification,
      Configuration::endpointPath(
          {Data.Segments.Souls, TxId, Data.Segments.Verify}),
      Request, Detail::EncodeSoulVerificationRequest,
      Detail::DecodeSoulVerifyResponse,
      {soulTagAdapter(TxId)});
}

} // namespace APISlice::Endpoints
