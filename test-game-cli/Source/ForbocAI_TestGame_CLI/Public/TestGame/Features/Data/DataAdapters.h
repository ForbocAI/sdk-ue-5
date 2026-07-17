#pragma once

#include "Core/fp.hpp"
#include "Containers/SharedString.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace TestGame::DataAdapters {

struct FSettingsSource {
  TSharedRef<FJsonObject> Root;
};

/** User Story: As a features data consumer, I need to invoke settings source key through a stable signature so the features data workflow remains explicit and composable. @fn inline FString SettingsSourceKey(const FString &Value) */
inline FString SettingsSourceKey(const FString &Value) { return Value; }

/** User Story: As a features data consumer, I need to invoke settings source through a stable signature so the features data workflow remains explicit and composable. @fn inline FSettingsSource SettingsSource(const FString &RelativePath) */
inline FSettingsSource SettingsSource(const FString &RelativePath) {
  const FString Path = FPaths::Combine(
      FPaths::ProjectContentDir(), SettingsSourceKey(TEXT("Data")),
      RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid());
  return {Root.ToSharedRef()};
}

/** User Story: As a features data consumer, I need to invoke read object field through a stable signature so the features data workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> ReadObjectField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TSharedRef<FJsonObject>
ReadObjectField(const TSharedRef<FJsonObject> &Object,
                const FString &Field) {
  return Object->GetObjectField(Field).ToSharedRef();
}

/** User Story: As a features data consumer, I need to invoke read object field through a stable signature so the features data workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> ReadObjectField(const FSettingsSource &Source, const FString &Field) */
inline TSharedRef<FJsonObject> ReadObjectField(const FSettingsSource &Source,
                                               const FString &Field) {
  return ReadObjectField(Source.Root, Field);
}

/** User Story: As a features data consumer, I need to invoke read object array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<TSharedPtr<FJsonValue>> ReadObjectArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<TSharedPtr<FJsonValue>>
ReadObjectArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return Object->GetArrayField(Field);
}

/** User Story: As a features data consumer, I need to invoke read string field through a stable signature so the features data workflow remains explicit and composable. @fn inline FString ReadStringField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline FString ReadStringField(const TSharedRef<FJsonObject> &Object,
                               const FString &Field) {
  return Object->GetStringField(Field);
}

/** User Story: As a features data consumer, I need to invoke read number field through a stable signature so the features data workflow remains explicit and composable. @fn inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return static_cast<int32>(Object->GetNumberField(Field));
}

/** User Story: As an authored-data consumer, I need fractional values decoded without truncation so typed runtime configuration preserves its source value. @fn inline float ReadFloatField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline float ReadFloatField(const TSharedRef<FJsonObject> &Object,
                            const FString &Field) {
  return static_cast<float>(Object->GetNumberField(Field));
}

/** User Story: As a features data consumer, I need to invoke read boolean field through a stable signature so the features data workflow remains explicit and composable. @fn inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->GetBoolField(Field);
}

/** User Story: As a features data consumer, I need to invoke read string array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<FString> ReadStringArrayField( const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<FString> ReadStringArrayField(
    const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, FString>(
      Object->GetArrayField(Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsString(); });
}

/**
 * User Story: As an authored-data consumer, I need keyed boolean state decoded
 * through a pure fold so coverage settings remain typed and order-independent.
 * @fn inline TMap<FString, bool> ReadBooleanMap(const TSharedRef<FJsonObject> &Values)
 */
inline TMap<FString, bool>
ReadBooleanMap(const TSharedRef<FJsonObject> &Values) {
  TArray<UE::FSharedString> AuthoredKeys;
  Values->Values.GetKeys(AuthoredKeys);
  const TArray<FString> Keys = func::map_array<UE::FSharedString, FString>(
      AuthoredKeys, [](const UE::FSharedString &Key) { return FString(*Key); });
  return func::fold_array<FString, TMap<FString, bool>>(
      Keys, {}, [&Values](const TMap<FString, bool> &Result,
                          const FString &Key) {
        return func::upsert_map_value<FString, bool>(
            Result, Key, false,
            [&Values, &Key](bool) { return Values->GetBoolField(Key); });
      });
}

} // namespace TestGame::DataAdapters
