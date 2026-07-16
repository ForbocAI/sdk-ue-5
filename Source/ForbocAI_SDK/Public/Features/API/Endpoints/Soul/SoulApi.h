#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"

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
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  return Detail::MakePostWithCodec<FSoulExportPreparationRequest,
                                   FSoulExportPreparation>(
      Data.Names.PostSoulExportPreparation,
      Configuration::apiEndpoint(Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Soul,
           Data.Segments.Export})),
      Request, Detail::EncodeSoulExportPreparationRequest,
      Detail::DecodeSoulExportPreparationResponse,
      {Configuration::endpointTag(TransportData.Tags.Soul, NpcId),
       Configuration::endpointTag(TransportData.Tags.Soul,
                                  Request.TransactionId)});
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
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  return Detail::MakePostWithCodec<FSoulExportConfirmationRequest,
                                   FSoulExportResponse>(
      Data.Names.PostSoulExportConfirmation,
      Configuration::apiEndpoint(Configuration::endpointPath(
          {Data.Segments.Npcs, NpcId, Data.Segments.Soul,
           Data.Segments.Confirm})),
      Request, Detail::EncodeSoulExportConfirmationRequest,
      Detail::DecodeSoulExportConfirmationResponse,
      {Configuration::endpointTag(TransportData.Tags.Soul,
                                  Request.TransactionId),
       Configuration::endpointTag(TransportData.Tags.Soul,
                                  Data.TagIds.List)});
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
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  return Detail::MakePostQueryWithCodec<FSoulVerificationRequest,
                                        FSoulVerifyResult>(
      Data.Names.PostSoulVerification,
      Configuration::apiEndpoint(Configuration::endpointPath(
          {Data.Segments.Souls, TxId, Data.Segments.Verify})),
      Request, Detail::EncodeSoulVerificationRequest,
      Detail::DecodeSoulVerifyResponse,
      {Configuration::endpointTag(TransportData.Tags.Soul, TxId)});
}

} // namespace APISlice::Endpoints
