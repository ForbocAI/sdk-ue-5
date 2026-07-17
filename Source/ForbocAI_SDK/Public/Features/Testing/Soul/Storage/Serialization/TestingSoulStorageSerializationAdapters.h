#pragma once

#include "Core/fp.hpp"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Features/Soul/Storage/Serialization/SoulStorageSerializationAdapters.h"
#include "Features/Testing/Soul/Storage/Serialization/SerializationTypes.h"

namespace Testing::Soul::Storage::Serialization {

/** User Story: As a Soul storage test, I need wallet fixtures decoded through the production-authored field map. @fn inline FSoulWallet readStorageWalletFixtureAdapter(const TSharedRef<FJsonObject> &Object) */
inline FSoulWallet
readStorageWalletFixtureAdapter(const TSharedRef<FJsonObject> &Object) {
  const SoulStorage::Configuration::FWalletFieldData &Fields =
      SoulStorage::Configuration::soulStorageData().Wallet.Fields;
  return {
      DataAdapters::ReadStringField(Object, Fields.KeyType),
      DataAdapters::ReadStringField(Object, Fields.PublicExponent),
      DataAdapters::ReadStringField(Object, Fields.Modulus),
      DataAdapters::ReadStringField(Object, Fields.PrivateExponent),
      DataAdapters::ReadStringField(Object, Fields.FirstPrimeFactor),
      DataAdapters::ReadStringField(Object, Fields.SecondPrimeFactor),
      DataAdapters::ReadStringField(Object, Fields.FirstFactorCrtExponent),
      DataAdapters::ReadStringField(Object, Fields.SecondFactorCrtExponent),
      DataAdapters::ReadStringField(Object, Fields.FirstCrtCoefficient),
  };
}

/** User Story: As a Soul storage test, I need envelope fixtures decoded through the production-authored field map. @fn inline FSoulEnvelope readStorageEnvelopeFixtureAdapter(const TSharedRef<FJsonObject> &Object) */
inline FSoulEnvelope
readStorageEnvelopeFixtureAdapter(const TSharedRef<FJsonObject> &Object) {
  const SoulStorage::Configuration::FEncryptionFieldData &Fields =
      SoulStorage::Configuration::soulStorageData().Encryption.Fields;
  return {
      DataAdapters::ReadStringField(Object, Fields.Version),
      DataAdapters::ReadStringField(Object, Fields.Algorithm),
      DataAdapters::ReadStringField(Object, Fields.KeyDerivation),
      DataAdapters::ReadStringField(Object, Fields.Salt),
      DataAdapters::ReadStringField(Object, Fields.InitializationVector),
      DataAdapters::ReadStringField(Object, Fields.Ciphertext),
  };
}

/** User Story: As a Soul storage test, I need catalog-entry fixtures decoded through the production-authored field map. @fn inline FSoulCatalogEntry readStorageCatalogEntryFixtureAdapter( const TSharedPtr<FJsonObject> &Object) */
inline FSoulCatalogEntry readStorageCatalogEntryFixtureAdapter(
    const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const SoulStorage::Configuration::FCatalogEntryFieldData &Fields =
      SoulStorage::Configuration::soulStorageData().Catalog.EntryFields;
  FSoulCatalogEntry Entry;
  Entry.TxId = DataAdapters::ReadStringField(Object.ToSharedRef(),
                                             Fields.TransactionId);
  Entry.Name = DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.Name);
  Entry.NpcId =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.NpcId);
  Entry.ExportedAt =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.ExportedAt);
  Entry.StorageUrl =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.StorageUrl);
  Entry.Provider =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.Provider);
  Entry.Digest =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.Digest);
  Entry.Signature =
      DataAdapters::ReadStringField(Object.ToSharedRef(), Fields.Signature);
  return Entry;
}

/** User Story: As a Soul storage test, I need catalog fixtures decoded through the production-authored field map. @fn inline FSoulCatalog readStorageCatalogFixtureAdapter(const TSharedRef<FJsonObject> &Object) */
inline FSoulCatalog
readStorageCatalogFixtureAdapter(const TSharedRef<FJsonObject> &Object) {
  const SoulStorage::Configuration::FCatalogFieldData &Fields =
      SoulStorage::Configuration::soulStorageData().Catalog.Fields;
  return {
      DataAdapters::ReadStringField(Object, Fields.Version),
      func::map_array<TSharedPtr<FJsonObject>, FSoulCatalogEntry>(
          DataAdapters::ReadObjectArrayField(Object, Fields.Souls),
          readStorageCatalogEntryFixtureAdapter),
  };
}

/** User Story: As a Soul storage test, I need one typed fixture assembled from authored settings. @fn inline FStorageSerializationFixture readStorageSerializationFixtureAdapter( const DataAdapters::FSettingsSource &Source) */
inline FStorageSerializationFixture readStorageSerializationFixtureAdapter(
    const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Storage =
      DataAdapters::ReadObjectField(Source, TEXT("storage"));
  return {
      readStorageWalletFixtureAdapter(
          DataAdapters::ReadObjectField(Storage, TEXT("wallet"))),
      readStorageEnvelopeFixtureAdapter(
          DataAdapters::ReadObjectField(Storage, TEXT("envelope"))),
      readStorageCatalogFixtureAdapter(
          DataAdapters::ReadObjectField(Storage, TEXT("catalog"))),
  };
}

/** User Story: As a Soul storage test, I need every wallet member compared after normalization. @fn inline bool storageWalletsEqualAdapter(const FSoulWallet &Left, const FSoulWallet &Right) */
inline bool storageWalletsEqualAdapter(const FSoulWallet &Left,
                                       const FSoulWallet &Right) {
  return Left.Kty == Right.Kty && Left.E == Right.E && Left.N == Right.N &&
         Left.D == Right.D && Left.P == Right.P && Left.Q == Right.Q &&
         Left.Dp == Right.Dp && Left.Dq == Right.Dq && Left.Qi == Right.Qi;
}

/** User Story: As a Soul storage test, I need every envelope member compared after normalization. @fn inline bool storageEnvelopesEqualAdapter(const FSoulEnvelope &Left, const FSoulEnvelope &Right) */
inline bool storageEnvelopesEqualAdapter(const FSoulEnvelope &Left,
                                         const FSoulEnvelope &Right) {
  return Left.Version == Right.Version && Left.Algorithm == Right.Algorithm &&
         Left.KeyDerivation == Right.KeyDerivation && Left.Salt == Right.Salt &&
         Left.InitializationVector == Right.InitializationVector &&
         Left.Ciphertext == Right.Ciphertext;
}

/** User Story: As a Soul storage test, I need every catalog-entry member compared after normalization. @fn inline bool storageCatalogEntriesEqualAdapter(const FSoulCatalogEntry &Left, const FSoulCatalogEntry &Right) */
inline bool storageCatalogEntriesEqualAdapter(const FSoulCatalogEntry &Left,
                                              const FSoulCatalogEntry &Right) {
  return Left.TxId == Right.TxId && Left.Name == Right.Name &&
         Left.NpcId == Right.NpcId && Left.ExportedAt == Right.ExportedAt &&
         Left.StorageUrl == Right.StorageUrl && Left.Provider == Right.Provider &&
         Left.Digest == Right.Digest && Left.Signature == Right.Signature;
}

/** User Story: As a Soul storage test, I need every catalog entry compared by index after normalization. @fn inline bool storageCatalogsEqualAdapter(const FSoulCatalog &Left, const FSoulCatalog &Right) */
inline bool storageCatalogsEqualAdapter(const FSoulCatalog &Left,
                                        const FSoulCatalog &Right) {
  return Left.Version == Right.Version && Left.Souls.Num() == Right.Souls.Num() &&
      func::fold_index_range<bool>(
          Left.Souls.Num(), true,
          [&Left, &Right](bool Equal, int32 Index) {
            return Equal && storageCatalogEntriesEqualAdapter(
                                Left.Souls[Index], Right.Souls[Index]);
          });
}

/** User Story: As a Soul storage test, I need typed round trips to prove every authored field survives UE serialization. @fn inline FStorageSerializationResult evaluateStorageSerializationAdapter( const FStorageSerializationFixture &Fixture) */
inline FStorageSerializationResult evaluateStorageSerializationAdapter(
    const FStorageSerializationFixture &Fixture) {
  FString Error;
  FSoulWallet Wallet;
  const bool bWalletDecoded =
      SoulStorage::Serialization::normalizeSoulWalletAdapter(
          SoulStorage::Serialization::encodeSoulWalletAdapter(Fixture.Wallet),
          Wallet, Error);

  FSoulEnvelope Envelope;
  const bool bEnvelopeDecoded =
      SoulStorage::Serialization::decodeSoulEnvelopeAdapter(
          SoulStorage::Serialization::encodeSoulEnvelopeAdapter(
              Fixture.Envelope),
          Envelope, Error);

  FSoulCatalog Catalog;
  const bool bCatalogDecoded =
      SoulStorage::Serialization::normalizeSoulCatalogAdapter(
          SoulStorage::Serialization::encodeSoulCatalogAdapter(Fixture.Catalog),
          Catalog, Error);

  return {
      bWalletDecoded && storageWalletsEqualAdapter(Fixture.Wallet, Wallet),
      bEnvelopeDecoded &&
          storageEnvelopesEqualAdapter(Fixture.Envelope, Envelope),
      bCatalogDecoded && storageCatalogsEqualAdapter(Fixture.Catalog, Catalog),
  };
}

} // namespace Testing::Soul::Storage::Serialization
