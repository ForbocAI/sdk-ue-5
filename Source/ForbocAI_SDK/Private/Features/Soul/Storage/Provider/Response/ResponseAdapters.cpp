#include "Features/Soul/Storage/Provider/Response/ResponseAdapters.h"

#include "Core/fp.hpp"
#include "Features/Soul/Storage/Configuration/ConfigurationAdapters.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace SoulStorage::Provider::Response {
namespace {

/**
 * User Story: As a provider receipt decoder, I need string arrays validated
 * without coercion before provider metadata enters SDK state.
 * @fn func::Maybe<TArray<FString>> decodeStringArrayAdapter( const TSharedPtr<FJsonObject> &Object, const FString &Field, bool bRequireValue)
 */
func::Maybe<TArray<FString>> decodeStringArrayAdapter(
    const TSharedPtr<FJsonObject> &Object, const FString &Field,
    bool bRequireValue) {
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  const bool bArray = Object.IsValid() &&
                      Object->TryGetArrayField(Field, Values) && Values;
  return !bArray
             ? func::nothing<TArray<FString>>()
             : [&]() {
                 const func::Maybe<TArray<FString>> Decoded =
                     func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                                FString>(
                         *Values,
                         [](const TSharedPtr<FJsonValue> &Value) {
                           return Value.IsValid() &&
                                          Value->Type == EJson::String
                                      ? func::just(Value->AsString())
                                      : func::nothing<FString>();
                         });
                 return func::is_nothing(Decoded) ||
                                (bRequireValue && Decoded.value.IsEmpty())
                            ? func::nothing<TArray<FString>>()
                            : Decoded;
               }();
}

} // namespace

/** User Story: As a storage provider response consumer, I need to invoke decode soul provider upload response adapter through a stable signature so the storage provider response workflow remains explicit and composable. @fn bool decodeSoulProviderUploadResponseAdapter( const FString &Json, FSoulProviderUploadResponse &Response) */
bool decodeSoulProviderUploadResponseAdapter(
    const FString &Json, FSoulProviderUploadResponse &Response) {
  const Configuration::FProviderResponseFieldData &Fields =
      Configuration::soulStorageData().Provider.ResponseFields;
  TSharedPtr<FJsonObject> Object;
  const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  const bool bRoot = FJsonSerializer::Deserialize(Reader, Object) &&
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Fields.Id) &&
      Object->HasTypedField<EJson::String>(Fields.Owner);
  const func::Maybe<TArray<FString>> DataCaches =
      bRoot ? decodeStringArrayAdapter(Object, Fields.DataCaches, true)
            : func::nothing<TArray<FString>>();
  const func::Maybe<TArray<FString>> Finality =
      bRoot ? decodeStringArrayAdapter(Object, Fields.FastFinalityIndexes,
                                       false)
            : func::nothing<TArray<FString>>();
  return func::is_nothing(DataCaches) || func::is_nothing(Finality)
             ? false
             : (Response.Id = Object->GetStringField(Fields.Id),
                Response.Owner = Object->GetStringField(Fields.Owner),
                Response.DataCaches = DataCaches.value,
                Response.FastFinalityIndexes = Finality.value, true);
}

} // namespace SoulStorage::Provider::Response
