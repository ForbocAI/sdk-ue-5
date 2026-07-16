#pragma once

#include "Core/fp.hpp"
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

} // namespace TestGame::DataAdapters
