#include "Systems/Soul/Storage/SoulStorageAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Crypto/Cipher/CipherAdapters.h"
#include "Systems/Soul/Storage/Crypto/DataItem/DataItemAdapters.h"
#include "Systems/Soul/Storage/Crypto/Wallet/WalletAdapters.h"
#include "Systems/Soul/Storage/Persistence/PersistenceAdapters.h"
#include "Systems/Soul/Storage/Serialization/SoulStorageSerializationAdapters.h"
#include "Misc/DateTime.h"
#include "Misc/ScopeLock.h"

namespace SoulStorage {
namespace {

FCriticalSection PendingSoulUploadsLock;
TMap<FString, FSoulPreparedUpload> PendingSoulUploads;

/** User Story: As a Soul storage adapter, I need synchronous failures lifted into the shared FP async error channel. @fn template <typename Value> func::AsyncResult<Value> rejectedSoulStorageAdapter(const FString &Error) */
template <typename Value>
func::AsyncResult<Value> rejectedSoulStorageAdapter(const FString &Error) {
  return rtk::detail::RejectAsync<Value>(Error);
}

/** User Story: As a Soul storage adapter, I need synchronous values lifted into the shared FP async success channel. @fn template <typename Value> func::AsyncResult<Value> resolvedSoulStorageAdapter(const Value &ValueToResolve) */
template <typename Value>
func::AsyncResult<Value>
resolvedSoulStorageAdapter(const Value &ValueToResolve) {
  return rtk::detail::ResolveAsync(ValueToResolve);
}

/** User Story: As every Soul cryptographic operation, I need one durable local wallet generated on first use and reused thereafter. @fn bool ensureSoulWalletAdapter(FSoulWallet &Wallet, FString &Error) */
bool ensureSoulWalletAdapter(FSoulWallet &Wallet, FString &Error) {
  bool bExists = false;
  const bool bRead =
      Persistence::readSoulWalletAdapter(Wallet, bExists, Error);
  return bRead &&
         (bExists ||
          (Crypto::Wallet::generateSoulWalletAdapter(Wallet, Error) &&
           Persistence::writeSoulWalletAdapter(Wallet, Error)));
}

/** User Story: As a Soul catalog caller, I need missing transaction failures composed from one authored message contract. @fn FString missingSoulCatalogEntryAdapter(const FString &TxId) */
FString missingSoulCatalogEntryAdapter(const FString &TxId) {
  const Configuration::FTextData &Text =
      Configuration::soulStorageData().Text;
  return Text.CatalogEntryMissingPrefix + TxId + Text.CatalogEntryMissingSuffix;
}

/** User Story: As a Soul catalog caller, I need one pure transaction lookup reused by import and verification. @fn func::Maybe<FSoulCatalogEntry> findSoulCatalogEntryAdapter(const FSoulCatalog &Catalog, const FString &TxId) */
func::Maybe<FSoulCatalogEntry>
findSoulCatalogEntryAdapter(const FSoulCatalog &Catalog, const FString &TxId) {
  return func::find_array<FSoulCatalogEntry>(
      Catalog.Souls,
      [&TxId](const FSoulCatalogEntry &Entry) { return Entry.TxId == TxId; });
}

/** User Story: As a Soul importer, I need retrieved envelope bytes authenticated, decrypted, and schema-normalized as one local boundary. @fn bool decodeDownloadedSoulAdapter(const TArray<uint8> &Payload, const FSoulWallet &Wallet, FSoul &Soul, FString &Error) */
bool decodeDownloadedSoulAdapter(const TArray<uint8> &Payload,
                                 const FSoulWallet &Wallet, FSoul &Soul,
                                 FString &Error) {
  FSoulEnvelope Envelope;
  TArray<uint8> Plaintext;
  return Serialization::decodeSoulEnvelopeAdapter(Payload, Envelope, Error) &&
         Crypto::Cipher::decryptSoulAdapter(Envelope, Wallet, Plaintext,
                                            Error) &&
         Serialization::normalizeSoulAdapter(Plaintext, Soul, Error);
}

} // namespace

/** User Story: As a features soul storage consumer, I need to invoke prepare soul storage adapter through a stable signature so the features soul storage workflow remains explicit and composable. @fn func::AsyncResult<FSoulStoragePreparation> prepareSoulStorageAdapter(const FSoul &Soul) */
func::AsyncResult<FSoulStoragePreparation>
prepareSoulStorageAdapter(const FSoul &Soul) {
  FSoulWallet Wallet;
  TArray<uint8> Plaintext;
  FSoulEnvelope Envelope;
  TArray<uint8> DataItem;
  FString Error;
  const bool bPrepared = ensureSoulWalletAdapter(Wallet, Error) &&
      Serialization::serializeSoulAdapter(Soul, Plaintext, Error) &&
      Crypto::Cipher::encryptSoulAdapter(Plaintext, Wallet, Envelope, Error);
  const TArray<uint8> Payload = bPrepared
      ? Serialization::encodeSoulEnvelopeAdapter(Envelope)
      : TArray<uint8>();
  const FString Digest = bPrepared
      ? Crypto::DataItem::soulPayloadDigestAdapter(Payload, Error)
      : FString();
  const bool bBuilt = bPrepared && !Digest.IsEmpty() &&
      Crypto::DataItem::buildSoulDataItemAdapter(Payload, Wallet, DataItem,
                                                 Error);
  const FString TxId = bBuilt
      ? Crypto::DataItem::soulDataItemIdAdapter(DataItem, Error)
      : FString();
  return TxId.IsEmpty()
             ? rejectedSoulStorageAdapter<FSoulStoragePreparation>(Error)
             : [&]() {
                 FScopeLock Lock(&PendingSoulUploadsLock);
                 PendingSoulUploads.Add(
                     TxId, FSoulPreparedUpload{DataItem, Digest});
                 return resolvedSoulStorageAdapter(FSoulStoragePreparation{
                     TxId, Digest,
                     Configuration::soulStorageData().Provider.Name});
               }();
}

/** User Story: As a Soul provider endpoint, I need prepared bytes loaded without removing them until upload and retrieval both succeed. @fn func::AsyncResult<FSoulPreparedUpload> requirePreparedSoulStorageAdapter(const FString &TxId) */
func::AsyncResult<FSoulPreparedUpload>
requirePreparedSoulStorageAdapter(const FString &TxId) {
  FScopeLock Lock(&PendingSoulUploadsLock);
  const func::Maybe<FSoulPreparedUpload> Found =
      func::from_nullable(PendingSoulUploads.Find(TxId));
  return func::match(
      Found,
      [](const FSoulPreparedUpload &Prepared) {
        return resolvedSoulStorageAdapter(Prepared);
      },
      []() {
        return rejectedSoulStorageAdapter<FSoulPreparedUpload>(
            Configuration::soulStorageData().Text.PendingUploadMissing);
      });
}

/** User Story: As a features soul storage consumer, I need to invoke discard soul storage adapter through a stable signature so the features soul storage workflow remains explicit and composable. @fn func::AsyncResult<rtk::FEmptyPayload> discardSoulStorageAdapter(const FString &TxId) */
func::AsyncResult<rtk::FEmptyPayload>
discardSoulStorageAdapter(const FString &TxId) {
  FScopeLock Lock(&PendingSoulUploadsLock);
  PendingSoulUploads.Remove(TxId);
  return resolvedSoulStorageAdapter(rtk::FEmptyPayload{});
}

/** User Story: As a features soul storage consumer, I need to invoke commit soul storage adapter through a stable signature so the features soul storage workflow remains explicit and composable. @fn func::AsyncResult<FSoulCatalogEntry> commitSoulStorageAdapter(const FSoulStorageCommit &Commit) */
func::AsyncResult<FSoulCatalogEntry>
commitSoulStorageAdapter(const FSoulStorageCommit &Commit) {
  FSoulCatalog Catalog;
  FString Error;
  const bool bRead = Persistence::readSoulCatalogAdapter(Catalog, Error);
  return !bRead
             ? rejectedSoulStorageAdapter<FSoulCatalogEntry>(Error)
             : [&]() {
                 FSoulCatalogEntry Entry;
                 Entry.TxId = Commit.Receipt.TxId;
                 Entry.Name = Commit.Soul.Name;
                 Entry.NpcId = Commit.Soul.Id;
                 Entry.ExportedAt = FDateTime::UtcNow().ToIso8601();
                 Entry.StorageUrl = Commit.Receipt.Url;
                 Entry.Provider = Commit.Receipt.Provider;
                 Entry.Digest = Commit.Receipt.Digest;
                 Entry.Signature = Commit.Signature;
                 Catalog.Souls = func::append_values<FSoulCatalogEntry>(
                     TArray<FSoulCatalogEntry>{Entry},
                     func::filter_array<FSoulCatalogEntry>(
                         Catalog.Souls,
                         [&Entry](const FSoulCatalogEntry &Current) {
                           return Current.TxId != Entry.TxId;
                         }));
                 return Persistence::writeSoulCatalogAdapter(Catalog, Error)
                            ? resolvedSoulStorageAdapter(Entry)
                            : rejectedSoulStorageAdapter<FSoulCatalogEntry>(
                                  Error);
               }();
}

/** User Story: As a Soul importer, I need retrieved provider bytes authenticated and decrypted locally without provider or API knowledge entering storage. @fn func::AsyncResult<FSoul> decryptSoulStoragePayloadAdapter(const TArray<uint8> &Payload) */
func::AsyncResult<FSoul>
decryptSoulStoragePayloadAdapter(const TArray<uint8> &Payload) {
  FSoulWallet Wallet;
  FSoul Soul;
  FString Error;
  return ensureSoulWalletAdapter(Wallet, Error) &&
                 decodeDownloadedSoulAdapter(Payload, Wallet, Soul, Error)
             ? resolvedSoulStorageAdapter(Soul)
             : rejectedSoulStorageAdapter<FSoul>(Error);
}

/** User Story: As a features soul storage consumer, I need to invoke list soul storage adapter through a stable signature so the features soul storage workflow remains explicit and composable. @fn func::AsyncResult<TArray<FSoulListItem>> listSoulStorageAdapter(int32 Limit) */
func::AsyncResult<TArray<FSoulListItem>>
listSoulStorageAdapter(int32 Limit) {
  FSoulCatalog Catalog;
  FString Error;
  const bool bRead = Persistence::readSoulCatalogAdapter(Catalog, Error);
  return !bRead
             ? rejectedSoulStorageAdapter<TArray<FSoulListItem>>(Error)
             : resolvedSoulStorageAdapter(
                   func::map_array<FSoulCatalogEntry, FSoulListItem>(
                       func::take_array(Catalog.Souls, FMath::Max(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, Limit)),
                       [](const FSoulCatalogEntry &Entry) { return Entry; }));
}

/** User Story: As a features soul storage consumer, I need to invoke get soul storage entry adapter through a stable signature so the features soul storage workflow remains explicit and composable. @fn func::AsyncResult<FSoulCatalogEntry> getSoulStorageEntryAdapter(const FString &TxId) */
func::AsyncResult<FSoulCatalogEntry>
getSoulStorageEntryAdapter(const FString &TxId) {
  FSoulCatalog Catalog;
  FString Error;
  const func::Maybe<FSoulCatalogEntry> Entry =
      Persistence::readSoulCatalogAdapter(Catalog, Error)
          ? findSoulCatalogEntryAdapter(Catalog, TxId)
          : func::nothing<FSoulCatalogEntry>();
  return func::is_nothing(Entry)
             ? rejectedSoulStorageAdapter<FSoulCatalogEntry>(
                   Error.IsEmpty() ? missingSoulCatalogEntryAdapter(TxId)
                                   : Error)
             : resolvedSoulStorageAdapter(Entry.value);
}

} // namespace SoulStorage
