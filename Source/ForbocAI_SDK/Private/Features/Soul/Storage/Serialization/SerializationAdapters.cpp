#include "Features/Soul/Storage/Serialization/SerializationAdapters.h"

#include "Core/JsonInterop.h"
#include "Features/API/Serialization/Soul/SerializationSoulAdapters.h"
#include "Features/Soul/Storage/Configuration/ConfigurationAdapters.h"
#include "Features/Soul/Storage/Crypto/Encoding/EncodingAdapters.h"

namespace SoulStorage::Serialization {
namespace {

/** User Story: As a persisted-wallet reader, I need all JWK strings present and nonempty before key reconstruction. @fn bool decodeSoulWalletObjectAdapter(const TSharedPtr<FJsonObject> &Object, FSoulWallet &Wallet) */
bool decodeSoulWalletObjectAdapter(const TSharedPtr<FJsonObject> &Object,
                                   FSoulWallet &Wallet) {
  const TArray<FString> Fields{TEXT("kty"), TEXT("e"), TEXT("n"),
                               TEXT("d"),   TEXT("p"), TEXT("q"),
                               TEXT("dp"),  TEXT("dq"), TEXT("qi")};
  const bool bComplete = Object.IsValid() &&
      func::fold_array<FString, bool>(
          Fields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  return !bComplete
             ? false
             : (Wallet.Kty = Object->GetStringField(TEXT("kty")),
                Wallet.E = Object->GetStringField(TEXT("e")),
                Wallet.N = Object->GetStringField(TEXT("n")),
                Wallet.D = Object->GetStringField(TEXT("d")),
                Wallet.P = Object->GetStringField(TEXT("p")),
                Wallet.Q = Object->GetStringField(TEXT("q")),
                Wallet.Dp = Object->GetStringField(TEXT("dp")),
                Wallet.Dq = Object->GetStringField(TEXT("dq")),
                Wallet.Qi = Object->GetStringField(TEXT("qi")), true);
}

/** User Story: As a Soul catalog writer, I need every confirmed entry projected into the stable durable shape. @fn TSharedPtr<FJsonValue> soulCatalogEntryValueAdapter(const FSoulCatalogEntry &Entry) */
TSharedPtr<FJsonValue>
soulCatalogEntryValueAdapter(const FSoulCatalogEntry &Entry) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(TEXT("txId"), Entry.TxId);
  Object->SetStringField(TEXT("name"), Entry.Name);
  Object->SetStringField(TEXT("npcId"), Entry.NpcId);
  Object->SetStringField(TEXT("exportedAt"), Entry.ExportedAt);
  Object->SetStringField(TEXT("storageUrl"), Entry.StorageUrl);
  Object->SetStringField(TEXT("provider"), Entry.Provider);
  Object->SetStringField(TEXT("digest"), Entry.Digest);
  Object->SetStringField(TEXT("signature"), Entry.Signature);
  return MakeShared<FJsonValueObject>(Object);
}

/** User Story: As a Soul catalog reader, I need each metadata entry validated atomically so malformed authorization cannot enter a partial catalog. @fn func::Maybe<FSoulCatalogEntry> decodeSoulCatalogEntryAdapter(const TSharedPtr<FJsonValue> &Value) */
func::Maybe<FSoulCatalogEntry>
decodeSoulCatalogEntryAdapter(const TSharedPtr<FJsonValue> &Value) {
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject()
                                                       : nullptr;
  const TArray<FString> Fields{TEXT("txId"),      TEXT("name"),
                               TEXT("npcId"),     TEXT("exportedAt"),
                               TEXT("storageUrl"), TEXT("provider"),
                               TEXT("digest"),    TEXT("signature")};
  const bool bComplete = Object.IsValid() &&
      func::fold_array<FString, bool>(
          Fields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  return !bComplete
             ? func::nothing<FSoulCatalogEntry>()
             : [&]() {
                 FSoulCatalogEntry Entry;
                 Entry.TxId = Object->GetStringField(TEXT("txId"));
                 Entry.Name = Object->GetStringField(TEXT("name"));
                 Entry.NpcId = Object->GetStringField(TEXT("npcId"));
                 Entry.ExportedAt =
                     Object->GetStringField(TEXT("exportedAt"));
                 Entry.StorageUrl =
                     Object->GetStringField(TEXT("storageUrl"));
                 Entry.Provider = Object->GetStringField(TEXT("provider"));
                 Entry.Digest = Object->GetStringField(TEXT("digest"));
                 Entry.Signature = Object->GetStringField(TEXT("signature"));
                 return func::just(Entry);
               }();
}

} // namespace

/** User Story: As a soul storage serialization consumer, I need to invoke serialize soul adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool serializeSoulAdapter(const FSoul &Soul, TArray<uint8> &Bytes, FString &Error) */
bool serializeSoulAdapter(const FSoul &Soul, TArray<uint8> &Bytes,
                          FString &Error) {
  const FString Json = JsonInterop::StringifyObject(
      JsonInterop::SoulToObject(Soul));
  Bytes = Crypto::Encoding::soulUtf8BytesAdapter(Json);
  Error = Json.IsEmpty() ? Configuration::soulStorageData().Text.InvalidSoul
                         : FString();
  return !Json.IsEmpty();
}

/** User Story: As a soul storage serialization consumer, I need to invoke normalize soul adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool normalizeSoulAdapter(const TArray<uint8> &Bytes, FSoul &Soul, FString &Error) */
bool normalizeSoulAdapter(const TArray<uint8> &Bytes, FSoul &Soul,
                          FString &Error) {
  TSharedPtr<FJsonObject> Object;
  const bool bParsed = JsonInterop::ParseJsonObject(
      Crypto::Encoding::soulUtf8StringAdapter(Bytes), Object);
  const func::Maybe<FSoul> Decoded =
      bParsed ? JsonInterop::DecodeSoulObject(Object)
              : func::nothing<FSoul>();
  Error = func::is_nothing(Decoded)
              ? Configuration::soulStorageData().Text.InvalidSoul
              : FString();
  return func::is_nothing(Decoded) ? false : (Soul = Decoded.value, true);
}

/** User Story: As a soul storage serialization consumer, I need to invoke encode soul envelope adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn TArray<uint8> encodeSoulEnvelopeAdapter(const FSoulEnvelope &Envelope) */
TArray<uint8> encodeSoulEnvelopeAdapter(const FSoulEnvelope &Envelope) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(TEXT("version"), Envelope.Version);
  Object->SetStringField(TEXT("algorithm"), Envelope.Algorithm);
  Object->SetStringField(TEXT("keyDerivation"), Envelope.KeyDerivation);
  Object->SetStringField(TEXT("salt"), Envelope.Salt);
  Object->SetStringField(TEXT("initializationVector"),
                         Envelope.InitializationVector);
  Object->SetStringField(TEXT("ciphertext"), Envelope.Ciphertext);
  return Crypto::Encoding::soulUtf8BytesAdapter(
      JsonInterop::StringifyObject(Object));
}

/** User Story: As a soul storage serialization consumer, I need to invoke decode soul envelope adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes, FSoulEnvelope &Envelope, FString &Error) */
bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes,
                               FSoulEnvelope &Envelope, FString &Error) {
  TSharedPtr<FJsonObject> Object;
  const TArray<FString> Fields{
      TEXT("version"), TEXT("algorithm"), TEXT("keyDerivation"),
      TEXT("salt"), TEXT("initializationVector"), TEXT("ciphertext")};
  const bool bParsed = JsonInterop::ParseJsonObject(
      Crypto::Encoding::soulUtf8StringAdapter(Bytes), Object);
  const bool bComplete = bParsed && Object.IsValid() &&
      func::fold_array<FString, bool>(
          Fields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  Error = bComplete ? FString()
                    : Configuration::soulStorageData().Text.InvalidEnvelope;
  return !bComplete
             ? false
             : (Envelope.Version = Object->GetStringField(TEXT("version")),
                Envelope.Algorithm = Object->GetStringField(TEXT("algorithm")),
                Envelope.KeyDerivation =
                    Object->GetStringField(TEXT("keyDerivation")),
                Envelope.Salt = Object->GetStringField(TEXT("salt")),
                Envelope.InitializationVector =
                    Object->GetStringField(TEXT("initializationVector")),
                Envelope.Ciphertext =
                    Object->GetStringField(TEXT("ciphertext")), true);
}

/** User Story: As a soul storage serialization consumer, I need to invoke encode soul wallet adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn FString encodeSoulWalletAdapter(const FSoulWallet &Wallet) */
FString encodeSoulWalletAdapter(const FSoulWallet &Wallet) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(TEXT("kty"), Wallet.Kty);
  Object->SetStringField(TEXT("e"), Wallet.E);
  Object->SetStringField(TEXT("n"), Wallet.N);
  Object->SetStringField(TEXT("d"), Wallet.D);
  Object->SetStringField(TEXT("p"), Wallet.P);
  Object->SetStringField(TEXT("q"), Wallet.Q);
  Object->SetStringField(TEXT("dp"), Wallet.Dp);
  Object->SetStringField(TEXT("dq"), Wallet.Dq);
  Object->SetStringField(TEXT("qi"), Wallet.Qi);
  return JsonInterop::StringifyObject(Object);
}

/** User Story: As a soul storage serialization consumer, I need to invoke normalize soul wallet adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool normalizeSoulWalletAdapter(const FString &Json, FSoulWallet &Wallet, FString &Error) */
bool normalizeSoulWalletAdapter(const FString &Json, FSoulWallet &Wallet,
                                FString &Error) {
  TSharedPtr<FJsonObject> Object;
  const bool bValid = JsonInterop::ParseJsonObject(Json, Object) &&
                      decodeSoulWalletObjectAdapter(Object, Wallet);
  Error = bValid ? FString()
                 : Configuration::soulStorageData().Text.InvalidWallet;
  return bValid;
}

/** User Story: As a soul storage serialization consumer, I need to invoke encode soul catalog adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn FString encodeSoulCatalogAdapter(const FSoulCatalog &Catalog) */
FString encodeSoulCatalogAdapter(const FSoulCatalog &Catalog) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(TEXT("version"), Catalog.Version);
  Object->SetArrayField(
      TEXT("souls"),
      func::map_array<FSoulCatalogEntry, TSharedPtr<FJsonValue>>(
          Catalog.Souls, soulCatalogEntryValueAdapter));
  return JsonInterop::StringifyObject(Object);
}

/** User Story: As a soul storage serialization consumer, I need to invoke normalize soul catalog adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool normalizeSoulCatalogAdapter(const FString &Json, FSoulCatalog &Catalog, FString &Error) */
bool normalizeSoulCatalogAdapter(const FString &Json, FSoulCatalog &Catalog,
                                 FString &Error) {
  TSharedPtr<FJsonObject> Object;
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  const FString Version = Configuration::soulStorageData().Text.CatalogVersion;
  const bool bRoot = JsonInterop::ParseJsonObject(Json, Object) &&
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(TEXT("version")) &&
      Object->GetStringField(TEXT("version")) == Version &&
      Object->TryGetArrayField(TEXT("souls"), Values) && Values;
  const func::Maybe<TArray<FSoulCatalogEntry>> Entries =
      bRoot ? func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                               FSoulCatalogEntry>(
                  *Values, decodeSoulCatalogEntryAdapter)
            : func::nothing<TArray<FSoulCatalogEntry>>();
  Error = func::is_nothing(Entries)
              ? Configuration::soulStorageData().Text.InvalidCatalog
              : FString();
  return func::is_nothing(Entries)
             ? false
             : (Catalog.Version = Version, Catalog.Souls = Entries.value,
                true);
}

} // namespace SoulStorage::Serialization
