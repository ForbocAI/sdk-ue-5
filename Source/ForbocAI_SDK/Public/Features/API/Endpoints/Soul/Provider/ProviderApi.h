#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"
#include "Features/API/Endpoints/Soul/Provider/ProviderAdapters.h"
#include "Features/API/Endpoints/Soul/Storage/StorageAdapters.h"
#include "Features/Soul/Storage/Configuration/ConfigurationAdapters.h"
#include "Features/Soul/Storage/Provider/ProviderAdapters.h"
#include "Features/Soul/Storage/StorageAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As a Soul exporter, I need upload and external digest verification represented by the package root RTK Query API. @fn inline Thunk<FSoulStorageReceipt> postSoulStorageUpload(const FString &TxId) */
inline Thunk<FSoulStorageReceipt>
postSoulStorageUpload(const FString &TxId) {
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  return Detail::MakeEndpoint<FSoulStorageTransactionRequest,
                              FSoulStorageReceipt>(
      TEXT("postSoulStorageUpload"), {TxId},
      [Data](const FSoulStorageTransactionRequest &Request) {
        const func::AsyncResult<FSoulStorageReceipt> Operation =
            func::AsyncChain::then<FSoulPreparedUpload,
                                   FSoulStorageReceipt>(
                ::SoulStorage::requirePreparedSoulStorageAdapter(Request.TxId),
                [Data, Request](const FSoulPreparedUpload &Pending) {
                  return func::AsyncChain::then<
                      SoulProviderEndpoint::FSoulProviderUpload,
                      FSoulStorageReceipt>(
                      SoulProviderEndpoint::
                          uploadSoulProviderWithRetryAdapter(
                              Pending, Request.TxId,
                              Data.Upload.InitialAttempt),
                      [Data, Pending, Request](
                          const SoulProviderEndpoint::FSoulProviderUpload
                              &Uploaded) {
                        const TArray<FString> Urls =
                            ::SoulStorage::Provider::soulProviderUrlsAdapter(
                                Uploaded.Response);
                        return func::AsyncChain::then<
                            SoulProviderEndpoint::FSoulProviderPayload,
                            FSoulStorageReceipt>(
                            SoulProviderEndpoint::
                                retrieveMatchingSoulPayloadAdapter(
                                    Urls, Pending.Digest,
                                    Data.Retrieval.InitialAttempt),
                            [Data, Pending, Request, Uploaded](
                                const SoulProviderEndpoint::
                                    FSoulProviderPayload &Retrieved) {
                              const FSoulStorageReceipt Receipt{
                                  Uploaded.Response.Id, Retrieved.Url,
                                  Pending.Digest, Uploaded.Status,
                                  Data.Provider.Name};
                              return func::AsyncChain::then<
                                  rtk::FEmptyPayload, FSoulStorageReceipt>(
                                  ::SoulStorage::discardSoulStorageAdapter(
                                      Request.TxId),
                                  [Receipt](const rtk::FEmptyPayload &) {
                                    return rtk::detail::ResolveAsync(Receipt);
                                  });
                            });
                      });
                });
        return SoulStorageEndpoint::storageQueryResult(Operation);
      },
      {}, {Configuration::endpointTag(TransportData.Tags.Soul)},
      rtk::DefinitionType::mutation);
}

/** User Story: As a Soul importer, I need provider retrieval and local authenticated decryption represented by the package root RTK Query API. @fn inline Thunk<FSoul> getSoulStorageDownload(const FString &TxId) */
inline Thunk<FSoul> getSoulStorageDownload(const FString &TxId) {
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  const int32 InitialAttempt =
      SoulStorage::Configuration::soulStorageData()
          .Retrieval.InitialAttempt;
  return Detail::MakeEndpoint<FSoulStorageTransactionRequest, FSoul>(
      TEXT("getSoulStorageDownload"), {TxId},
      [InitialAttempt](const FSoulStorageTransactionRequest &Request) {
        const func::AsyncResult<FSoul> Operation = func::AsyncChain::then<
            FSoulCatalogEntry, FSoul>(
            ::SoulStorage::getSoulStorageEntryAdapter(Request.TxId),
            [InitialAttempt](const FSoulCatalogEntry &Entry) {
              return func::AsyncChain::then<
                  SoulProviderEndpoint::FSoulProviderPayload, FSoul>(
                  SoulProviderEndpoint::retrieveMatchingSoulPayloadAdapter(
                      TArray<FString>{Entry.StorageUrl}, Entry.Digest,
                      InitialAttempt),
                  [](const SoulProviderEndpoint::FSoulProviderPayload
                         &Retrieved) {
                    return ::SoulStorage::decryptSoulStoragePayloadAdapter(
                        Retrieved.Payload);
                  });
            });
        return SoulStorageEndpoint::storageQueryResult(Operation);
      },
      {Configuration::endpointTag(TransportData.Tags.Soul, TxId)});
}

/** User Story: As a Soul owner, I need provider retrieval and authenticated decryption reported as RTK Query verification data rather than a rejected validity query. @fn inline Thunk<FSoulVerifyResult> getSoulStorageVerification(const FString &TxId) */
inline Thunk<FSoulVerifyResult>
getSoulStorageVerification(const FString &TxId) {
  const Transport::FTransportQueryData &TransportData =
      Transport::transportQueryData();
  const int32 InitialAttempt =
      SoulStorage::Configuration::soulStorageData()
          .Retrieval.InitialAttempt;
  return Detail::MakeEndpoint<FSoulStorageTransactionRequest,
                              FSoulVerifyResult>(
      TEXT("getSoulStorageVerification"), {TxId},
      [InitialAttempt](const FSoulStorageTransactionRequest &Request) {
        const func::AsyncResult<FSoul> Download = func::AsyncChain::then<
            FSoulCatalogEntry, FSoul>(
            ::SoulStorage::getSoulStorageEntryAdapter(Request.TxId),
            [InitialAttempt](const FSoulCatalogEntry &Entry) {
              return func::AsyncChain::then<
                  SoulProviderEndpoint::FSoulProviderPayload, FSoul>(
                  SoulProviderEndpoint::retrieveMatchingSoulPayloadAdapter(
                      TArray<FString>{Entry.StorageUrl}, Entry.Digest,
                      InitialAttempt),
                  [](const SoulProviderEndpoint::FSoulProviderPayload
                         &Retrieved) {
                    return ::SoulStorage::decryptSoulStoragePayloadAdapter(
                        Retrieved.Payload);
                  });
            });
        const func::AsyncResult<FSoulVerifyResult> Operation =
            func::AsyncResult<FSoulVerifyResult>::create(
                [Download](std::function<void(FSoulVerifyResult)> Resolve,
                           std::function<void(std::string)> Reject) {
                  (void)Reject;
                  Download
                      .then([Resolve](const FSoul &) {
                        Resolve(FSoulVerifyResult{true, FString()});
                      })
                      .catch_([Resolve](const std::string &Error) {
                        Resolve(FSoulVerifyResult{
                            false, FString(UTF8_TO_TCHAR(Error.c_str()))});
                      })
                      .execute();
                });
        return SoulStorageEndpoint::storageQueryResult(Operation);
      },
      {Configuration::endpointTag(TransportData.Tags.Soul, TxId)});
}

} // namespace APISlice::Endpoints
