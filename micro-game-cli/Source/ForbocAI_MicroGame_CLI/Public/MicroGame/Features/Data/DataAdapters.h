#pragma once

#include "Core/fp.hpp"
#include "Containers/SharedString.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MicroGame::DataAdapters {

struct FSettingsSource {
  TSharedRef<FJsonObject> Root;
};

struct FArraySource {
  TArray<TSharedPtr<FJsonValue>> Root;
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

/** User Story: As a features data consumer, I need root arrays loaded through the same authored-data boundary as object settings. @fn inline FArraySource ArraySource(const FString &RelativePath) */
inline FArraySource ArraySource(const FString &RelativePath) {
  const FString Path = FPaths::Combine(
      FPaths::ProjectContentDir(), SettingsSourceKey(TEXT("Data")),
      RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TArray<TSharedPtr<FJsonValue>> Root;
  const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Root));
  return {Root};
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

/**
 * User Story: As a features data consumer, I need to invoke read object array field through a stable signature so the features data workflow remains explicit and composable.
 * @fn inline TArray<TSharedPtr<FJsonValue>> ReadArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field)
 */
inline TArray<TSharedPtr<FJsonValue>>
ReadArrayField(const TSharedRef<FJsonObject> &Object,
               const FString &Field) {
  return Object->GetArrayField(Field);
}

/** User Story: As a features data consumer, I need arrays decoded through one schema adapter so nested authored data remains independent from behavior code. @fn inline TArray<TSharedPtr<FJsonValue>> ReadObjectArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<TSharedPtr<FJsonValue>>
ReadObjectArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return ReadArrayField(Object, Field);
}

/** User Story: As a features data consumer, I need root object arrays decoded through one typed adapter. @fn inline TArray<TSharedPtr<FJsonObject>> ReadObjectArray(const FArraySource &Source) */
inline TArray<TSharedPtr<FJsonObject>>
ReadObjectArray(const FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonValue>, TSharedPtr<FJsonObject>>(
      Source.Root,
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsObject(); });
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

/** User Story: As a quality report reader, I need report precision retained without narrowing fractional evidence to float. @fn inline double ReadDoubleField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline double ReadDoubleField(const TSharedRef<FJsonObject> &Object,
                              const FString &Field) {
  return Object->GetNumberField(Field);
}

/** User Story: As a quality report reader, I need nullable strings decoded without sentinel values. @fn inline FString ReadOptionalStringField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline FString ReadOptionalStringField(const TSharedRef<FJsonObject> &Object,
                                       const FString &Field) {
  FString Value;
  return Object->TryGetStringField(Field, Value) ? Value : FString();
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

/** User Story: As a report serializer, I need string schema fields written through one typed JSON boundary. @fn inline void WriteStringField(const TSharedRef<FJsonObject> &Object, const FString &Field, const FString &Value) */
inline void WriteStringField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field, const FString &Value) {
  Object->SetStringField(Field, Value);
}

/** User Story: As a report serializer, I need numeric schema fields written through one typed JSON boundary. @fn inline void WriteNumberField(const TSharedRef<FJsonObject> &Object, const FString &Field, double Value) */
inline void WriteNumberField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field, double Value) {
  Object->SetNumberField(Field, Value);
}

/** User Story: As a report serializer, I need boolean schema fields written through one typed JSON boundary. @fn inline void WriteBooleanField(const TSharedRef<FJsonObject> &Object, const FString &Field, bool bValue) */
inline void WriteBooleanField(const TSharedRef<FJsonObject> &Object,
                              const FString &Field, bool bValue) {
  Object->SetBoolField(Field, bValue);
}

/** User Story: As a report serializer, I need nested object schema fields written through one typed JSON boundary. @fn inline void WriteObjectField(const TSharedRef<FJsonObject> &Object, const FString &Field, const TSharedRef<FJsonObject> &Value) */
inline void WriteObjectField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field,
                             const TSharedRef<FJsonObject> &Value) {
  Object->SetObjectField(Field, Value);
}

/** User Story: As a report serializer, I need array schema fields written through one typed JSON boundary. @fn inline void WriteArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field, const TArray<TSharedPtr<FJsonValue>> &Value) */
inline void WriteArrayField(const TSharedRef<FJsonObject> &Object,
                            const FString &Field,
                            const TArray<TSharedPtr<FJsonValue>> &Value) {
  Object->SetArrayField(Field, Value);
}

/** User Story: As a report serializer, I need absent optional values encoded explicitly rather than omitted or replaced with sentinels. @fn inline void WriteNullField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline void WriteNullField(const TSharedRef<FJsonObject> &Object,
                           const FString &Field) {
  Object->SetField(Field, MakeShared<FJsonValueNull>());
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

/** User Story: As an authored-data consumer, I need keyed strings decoded through a pure fold so relationship maps remain JSON-owned. @fn inline TMap<FString, FString> ReadStringMap(const TSharedRef<FJsonObject> &Values) */
inline TMap<FString, FString>
ReadStringMap(const TSharedRef<FJsonObject> &Values) {
  TArray<UE::FSharedString> AuthoredKeys;
  Values->Values.GetKeys(AuthoredKeys);
  const TArray<FString> Keys = func::map_array<UE::FSharedString, FString>(
      AuthoredKeys, [](const UE::FSharedString &Key) { return FString(*Key); });
  return func::fold_array<FString, TMap<FString, FString>>(
      Keys, {}, [&Values](const TMap<FString, FString> &Result,
                          const FString &Key) {
        return func::upsert_map_value<FString, FString>(
            Result, Key, FString(),
            [&Values, &Key](const FString &) {
              return Values->GetStringField(Key);
            });
      });
}

} // namespace MicroGame::DataAdapters
