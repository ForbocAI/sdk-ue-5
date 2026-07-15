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

inline FString SettingsSourceKey(const FString &Value) { return Value; }

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

inline TSharedRef<FJsonObject>
ReadObjectField(const TSharedRef<FJsonObject> &Object,
                const FString &Field) {
  return Object->GetObjectField(Field).ToSharedRef();
}

inline TSharedRef<FJsonObject> ReadObjectField(const FSettingsSource &Source,
                                               const FString &Field) {
  return ReadObjectField(Source.Root, Field);
}

inline TArray<TSharedPtr<FJsonValue>>
ReadObjectArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return Object->GetArrayField(Field);
}

inline FString ReadStringField(const TSharedRef<FJsonObject> &Object,
                               const FString &Field) {
  return Object->GetStringField(Field);
}

inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return static_cast<int32>(Object->GetNumberField(Field));
}

inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->GetBoolField(Field);
}

inline TArray<FString> ReadStringArrayField(
    const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, FString>(
      Object->GetArrayField(Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsString(); });
}

} // namespace TestGame::DataAdapters
