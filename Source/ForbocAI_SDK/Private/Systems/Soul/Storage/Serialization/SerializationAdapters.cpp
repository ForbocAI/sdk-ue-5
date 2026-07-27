#include "Systems/Soul/Storage/Serialization/SoulStorageSerializationAdapters.h"

#include "Core/JsonInterop.h"
#include "Systems/API/Serialization/Soul/SerializationSoulAdapters.h"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Crypto/Encoding/EncodingAdapters.h"

namespace SoulStorage::Serialization {
namespace {

/** User Story: As a persisted-wallet reader, I need all JWK strings present and nonempty before key reconstruction. @fn bool decodeSoulWalletObjectAdapter(const TSharedPtr<FJsonObject> &Object, FSoulWallet &Wallet) */
bool decodeSoulWalletObjectAdapter(const TSharedPtr<FJsonObject> &Object,
                                   FSoulWallet &Wallet) {
  const Configuration::FWalletFieldData &Fields =
      Configuration::soulStorageData().Wallet.Fields;
  const TArray<FString> RequiredFields{
      Fields.KeyType,
      Fields.PublicExponent,
      Fields.Modulus,
      Fields.PrivateExponent,
      Fields.FirstPrimeFactor,
      Fields.SecondPrimeFactor,
      Fields.FirstFactorCrtExponent,
      Fields.SecondFactorCrtExponent,
      Fields.FirstCrtCoefficient,
  };
  const bool bComplete = Object.IsValid() &&
      func::fold_array<FString, bool>(
          RequiredFields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  return !bComplete
             ? false
             : (Wallet.Kty = Object->GetStringField(Fields.KeyType),
                Wallet.E = Object->GetStringField(Fields.PublicExponent),
                Wallet.N = Object->GetStringField(Fields.Modulus),
                Wallet.D = Object->GetStringField(Fields.PrivateExponent),
                Wallet.P = Object->GetStringField(Fields.FirstPrimeFactor),
                Wallet.Q = Object->GetStringField(Fields.SecondPrimeFactor),
                Wallet.Dp =
                    Object->GetStringField(Fields.FirstFactorCrtExponent),
                Wallet.Dq =
                    Object->GetStringField(Fields.SecondFactorCrtExponent),
                Wallet.Qi =
                    Object->GetStringField(Fields.FirstCrtCoefficient),
                true);
}

/** User Story: As a Soul catalog writer, I need every confirmed entry projected into the stable durable shape. @fn TSharedPtr<FJsonValue> soulCatalogEntryValueAdapter(const FSoulCatalogEntry &Entry) */
TSharedPtr<FJsonValue>
soulCatalogEntryValueAdapter(const FSoulCatalogEntry &Entry) {
  const Configuration::FCatalogEntryFieldData &Fields =
      Configuration::soulStorageData().Catalog.EntryFields;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.TransactionId, Entry.TxId);
  Object->SetStringField(Fields.Name, Entry.Name);
  Object->SetStringField(Fields.NpcId, Entry.NpcId);
  Object->SetStringField(Fields.ExportedAt, Entry.ExportedAt);
  Object->SetStringField(Fields.StorageUrl, Entry.StorageUrl);
  Object->SetStringField(Fields.Provider, Entry.Provider);
  Object->SetStringField(Fields.Digest, Entry.Digest);
  Object->SetStringField(Fields.Signature, Entry.Signature);
  return MakeShared<FJsonValueObject>(Object);
}

/** User Story: As a Soul catalog reader, I need each metadata entry validated atomically so malformed authorization cannot enter a partial catalog. @fn func::Maybe<FSoulCatalogEntry> decodeSoulCatalogEntryAdapter(const TSharedPtr<FJsonValue> &Value) */
func::Maybe<FSoulCatalogEntry>
decodeSoulCatalogEntryAdapter(const TSharedPtr<FJsonValue> &Value) {
  const Configuration::FCatalogEntryFieldData &Fields =
      Configuration::soulStorageData().Catalog.EntryFields;
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object ? Value->AsObject()
                                                       : nullptr;
  const TArray<FString> RequiredFields{
      Fields.TransactionId, Fields.Name,     Fields.NpcId,   Fields.ExportedAt,
      Fields.StorageUrl,    Fields.Provider, Fields.Digest,  Fields.Signature,
  };
  const bool bComplete = Object.IsValid() &&
      func::fold_array<FString, bool>(
          RequiredFields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  return !bComplete
             ? func::nothing<FSoulCatalogEntry>()
             : [&]() {
                 FSoulCatalogEntry Entry;
                 Entry.TxId = Object->GetStringField(Fields.TransactionId);
                 Entry.Name = Object->GetStringField(Fields.Name);
                 Entry.NpcId = Object->GetStringField(Fields.NpcId);
                 Entry.ExportedAt =
                     Object->GetStringField(Fields.ExportedAt);
                 Entry.StorageUrl =
                     Object->GetStringField(Fields.StorageUrl);
                 Entry.Provider = Object->GetStringField(Fields.Provider);
                 Entry.Digest = Object->GetStringField(Fields.Digest);
                 Entry.Signature = Object->GetStringField(Fields.Signature);
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
  const Configuration::FEncryptionFieldData &Fields =
      Configuration::soulStorageData().Encryption.Fields;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.Version, Envelope.Version);
  Object->SetStringField(Fields.Algorithm, Envelope.Algorithm);
  Object->SetStringField(Fields.KeyDerivation, Envelope.KeyDerivation);
  Object->SetStringField(Fields.Salt, Envelope.Salt);
  Object->SetStringField(Fields.InitializationVector,
                         Envelope.InitializationVector);
  Object->SetStringField(Fields.Ciphertext, Envelope.Ciphertext);
  return Crypto::Encoding::soulUtf8BytesAdapter(
      JsonInterop::StringifyObject(Object));
}

/** User Story: As a soul storage serialization consumer, I need to invoke decode soul envelope adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes, FSoulEnvelope &Envelope, FString &Error) */
bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes,
                               FSoulEnvelope &Envelope, FString &Error) {
  const Configuration::FEncryptionFieldData &Fields =
      Configuration::soulStorageData().Encryption.Fields;
  TSharedPtr<FJsonObject> Object;
  const TArray<FString> RequiredFields{
      Fields.Version, Fields.Algorithm, Fields.KeyDerivation,
      Fields.Salt,    Fields.InitializationVector, Fields.Ciphertext,
  };
  const bool bParsed = JsonInterop::ParseJsonObject(
      Crypto::Encoding::soulUtf8StringAdapter(Bytes), Object);
  const bool bComplete = bParsed && Object.IsValid() &&
      func::fold_array<FString, bool>(
          RequiredFields, true, [&Object](bool Valid, const FString &Field) {
            return Valid && Object->HasTypedField<EJson::String>(Field) &&
                   !Object->GetStringField(Field).IsEmpty();
          });
  Error = bComplete ? FString()
                    : Configuration::soulStorageData().Text.InvalidEnvelope;
  return !bComplete
             ? false
             : (Envelope.Version = Object->GetStringField(Fields.Version),
                Envelope.Algorithm = Object->GetStringField(Fields.Algorithm),
                Envelope.KeyDerivation =
                    Object->GetStringField(Fields.KeyDerivation),
                Envelope.Salt = Object->GetStringField(Fields.Salt),
                Envelope.InitializationVector =
                    Object->GetStringField(Fields.InitializationVector),
                Envelope.Ciphertext =
                    Object->GetStringField(Fields.Ciphertext), true);
}

/** User Story: As a soul storage serialization consumer, I need to invoke encode soul wallet adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn FString encodeSoulWalletAdapter(const FSoulWallet &Wallet) */
FString encodeSoulWalletAdapter(const FSoulWallet &Wallet) {
  const Configuration::FWalletFieldData &Fields =
      Configuration::soulStorageData().Wallet.Fields;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.KeyType, Wallet.Kty);
  Object->SetStringField(Fields.PublicExponent, Wallet.E);
  Object->SetStringField(Fields.Modulus, Wallet.N);
  Object->SetStringField(Fields.PrivateExponent, Wallet.D);
  Object->SetStringField(Fields.FirstPrimeFactor, Wallet.P);
  Object->SetStringField(Fields.SecondPrimeFactor, Wallet.Q);
  Object->SetStringField(Fields.FirstFactorCrtExponent, Wallet.Dp);
  Object->SetStringField(Fields.SecondFactorCrtExponent, Wallet.Dq);
  Object->SetStringField(Fields.FirstCrtCoefficient, Wallet.Qi);
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
  const Configuration::FCatalogFieldData &Fields =
      Configuration::soulStorageData().Catalog.Fields;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.Version, Catalog.Version);
  Object->SetArrayField(
      Fields.Souls,
      func::map_array<FSoulCatalogEntry, TSharedPtr<FJsonValue>>(
          Catalog.Souls, soulCatalogEntryValueAdapter));
  return JsonInterop::StringifyObject(Object);
}

/** User Story: As a soul storage serialization consumer, I need to invoke normalize soul catalog adapter through a stable signature so the soul storage serialization workflow remains explicit and composable. @fn bool normalizeSoulCatalogAdapter(const FString &Json, FSoulCatalog &Catalog, FString &Error) */
bool normalizeSoulCatalogAdapter(const FString &Json, FSoulCatalog &Catalog,
                                 FString &Error) {
  const Configuration::FCatalogFieldData &Fields =
      Configuration::soulStorageData().Catalog.Fields;
  TSharedPtr<FJsonObject> Object;
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  const FString Version = Configuration::soulStorageData().Text.CatalogVersion;
  const bool bRoot = JsonInterop::ParseJsonObject(Json, Object) &&
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Fields.Version) &&
      Object->GetStringField(Fields.Version) == Version &&
      Object->TryGetArrayField(Fields.Souls, Values) && Values;
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
