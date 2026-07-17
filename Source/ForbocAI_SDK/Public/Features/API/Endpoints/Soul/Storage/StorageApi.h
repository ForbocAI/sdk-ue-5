#pragma once

#include "Features/API/APIAdapters.h"
#include "Features/API/Endpoints/EndpointsTypes.h"
#include "Features/API/Endpoints/Soul/Storage/EndpointsSoulStorageAdapters.h"
#include "Features/Soul/Storage/SoulStorageAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As a Soul exporter, I need local preparation represented as an RTK Query mutation before API authorization. @fn inline Thunk<FSoulStoragePreparation> postSoulStoragePreparation(const FSoul &Soul) */
inline Thunk<FSoulStoragePreparation>
postSoulStoragePreparation(const FSoul &Soul) {
  const Configuration::FEndpointConfigurationData &EndpointData =
      Configuration::endpointData();
  return Detail::MakeEndpoint<FSoul, FSoulStoragePreparation>(
      EndpointData.Names.PostSoulStoragePreparation, Soul,
      [](const FSoul &Value, const rtk::ApiContext<FRuntimeState> &) {
        return SoulStorageEndpoint::storageQueryResult(
            ::SoulStorage::prepareSoulStorageAdapter(Value));
      },
      {}, {soulTagAdapter(Soul.Id)},
      rtk::DefinitionType::mutation);
}

/** User Story: As a failed Soul export, I need process-local prepared bytes discarded through an RTK Query mutation. @fn inline Thunk<rtk::FEmptyPayload> deleteSoulStoragePreparation(const FString &TxId) */
inline Thunk<rtk::FEmptyPayload>
deleteSoulStoragePreparation(const FString &TxId) {
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  return Detail::MakeEndpoint<FSoulStorageTransactionRequest,
                              rtk::FEmptyPayload>(
      Data.Names.DeleteSoulStoragePreparation, {TxId},
      [](const FSoulStorageTransactionRequest &Request,
         const rtk::ApiContext<FRuntimeState> &) {
        return SoulStorageEndpoint::storageQueryResult(
            ::SoulStorage::discardSoulStorageAdapter(Request.TxId));
      },
      {}, {soulTagAdapter(TxId)},
      rtk::DefinitionType::mutation);
}

/** User Story: As a confirmed Soul exporter, I need durable catalog publication represented as an RTK Query mutation. @fn inline Thunk<FSoulCatalogEntry> postSoulStorageCommit(const FSoulStorageCommit &Commit) */
inline Thunk<FSoulCatalogEntry>
postSoulStorageCommit(const FSoulStorageCommit &Commit) {
  const Configuration::FEndpointConfigurationData &EndpointData =
      Configuration::endpointData();
  return Detail::MakeEndpoint<FSoulStorageCommit, FSoulCatalogEntry>(
      EndpointData.Names.PostSoulStorageCommit, Commit,
      [](const FSoulStorageCommit &Value,
         const rtk::ApiContext<FRuntimeState> &) {
        return SoulStorageEndpoint::storageQueryResult(
            ::SoulStorage::commitSoulStorageAdapter(Value));
      },
      {}, {soulListTagAdapter(), soulTagAdapter(Commit.Receipt.TxId)},
      rtk::DefinitionType::mutation);
}

/** User Story: As a Soul browser, I need package-owned durable entries represented as an RTK Query catalog query. @fn inline Thunk<TArray<FSoulListItem>> getSoulStorageCatalog(int32 Limit) */
inline Thunk<TArray<FSoulListItem>> getSoulStorageCatalog(int32 Limit) {
  const Configuration::FEndpointConfigurationData &EndpointData =
      Configuration::endpointData();
  return Detail::MakeEndpoint<FSoulStorageListRequest,
                              TArray<FSoulListItem>>(
      EndpointData.Names.GetSoulStorageCatalog, {Limit},
      [](const FSoulStorageListRequest &Request,
         const rtk::ApiContext<FRuntimeState> &) {
        return SoulStorageEndpoint::storageQueryResult(
            ::SoulStorage::listSoulStorageAdapter(Request.Limit));
      },
      {soulListTagAdapter()});
}

/** User Story: As a Soul importer, I need local catalog entry reads represented as an RTK Query query. @fn inline Thunk<FSoulCatalogEntry> getSoulStorageEntry(const FString &TxId) */
inline Thunk<FSoulCatalogEntry>
getSoulStorageEntry(const FString &TxId) {
  const Configuration::FEndpointConfigurationData &EndpointData =
      Configuration::endpointData();
  return Detail::MakeEndpoint<FSoulStorageTransactionRequest,
                              FSoulCatalogEntry>(
      EndpointData.Names.GetSoulStorageEntry, {TxId},
      [](const FSoulStorageTransactionRequest &Request,
         const rtk::ApiContext<FRuntimeState> &) {
        return SoulStorageEndpoint::storageQueryResult(
            ::SoulStorage::getSoulStorageEntryAdapter(Request.TxId));
      },
      {soulTagAdapter(TxId)});
}

} // namespace APISlice::Endpoints
