#pragma once

#include "Core/fp.hpp"
#include "Dom/JsonObject.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace DataAdapters {

struct FSettingsSource {
  TSharedRef<FJsonObject> Root;
};

struct FArraySource {
  TArray<TSharedPtr<FJsonValue>> Root;
};

inline FSettingsSource SettingsSource(const FString &PluginName,
                                      const FString &RelativePath) {
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(PluginName);
  check(Plugin.IsValid());
  const FString Path =
      FPaths::Combine(Plugin->GetContentDir(), RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid());
  return {Root.ToSharedRef()};
}

inline FArraySource ArraySource(const FString &PluginName,
                                const FString &RelativePath) {
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(PluginName);
  check(Plugin.IsValid());
  const FString Path =
      FPaths::Combine(Plugin->GetContentDir(), RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TArray<TSharedPtr<FJsonValue>> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Root));
  return {Root};
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

inline FString ReadStringField(const TSharedRef<FJsonObject> &Object,
                               const FString &Field) {
  return Object->GetStringField(Field);
}

inline func::Maybe<FString>
ReadOptionalStringField(const TSharedRef<FJsonObject> &Object,
                        const FString &Field) {
  FString Value;
  return Object->TryGetStringField(Field, Value)
             ? func::just<FString>(Value)
             : func::nothing<FString>();
}

inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return static_cast<int32>(Object->GetNumberField(Field));
}

inline func::Maybe<int32>
ReadOptionalNumberField(const TSharedRef<FJsonObject> &Object,
                        const FString &Field) {
  double Value;
  return Object->TryGetNumberField(Field, Value)
             ? func::just<int32>(static_cast<int32>(Value))
             : func::nothing<int32>();
}

inline float ReadFloatField(const TSharedRef<FJsonObject> &Object,
                            const FString &Field) {
  return static_cast<float>(Object->GetNumberField(Field));
}

inline func::Maybe<float>
ReadOptionalFloatField(const TSharedRef<FJsonObject> &Object,
                       const FString &Field) {
  double Value;
  return Object->TryGetNumberField(Field, Value)
             ? func::just<float>(static_cast<float>(Value))
             : func::nothing<float>();
}

inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->GetBoolField(Field);
}

inline func::Maybe<bool>
ReadOptionalBooleanField(const TSharedRef<FJsonObject> &Object,
                         const FString &Field) {
  bool Value;
  return Object->TryGetBoolField(Field, Value) ? func::just<bool>(Value)
                                               : func::nothing<bool>();
}

inline TArray<int32> ReadNumberArrayField(
    const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, int32>(
      Object->GetArrayField(Field),
      [](const TSharedPtr<FJsonValue> &Value) {
        return static_cast<int32>(Value->AsNumber());
      });
}

inline TArray<TSharedPtr<FJsonValue>>
ReadArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return Object->GetArrayField(Field);
}

inline TArray<FString>
ReadStringArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, FString>(
      ReadArrayField(Object, Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsString(); });
}

inline func::Maybe<TArray<FString>>
ReadOptionalStringArrayField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->HasField(Field)
             ? func::just<TArray<FString>>(ReadStringArrayField(Object, Field))
             : func::nothing<TArray<FString>>();
}

inline TArray<TSharedPtr<FJsonObject>>
ReadObjectArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, TSharedPtr<FJsonObject>>(
      ReadArrayField(Object, Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsObject(); });
}

inline TArray<TSharedPtr<FJsonObject>>
ReadObjectArray(const FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonValue>, TSharedPtr<FJsonObject>>(
      Source.Root,
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsObject(); });
}

inline std::vector<int>
ReadNumberVectorField(const TSharedRef<FJsonObject> &Object,
                      const FString &Field) {
  const TArray<int32> Values = ReadNumberArrayField(Object, Field);
  return std::vector<int>(Values.begin(), Values.end());
}

inline FString SerializeObject(const TSharedRef<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&Json);
  check(FJsonSerializer::Serialize(Object, Writer));
  return Json;
}

inline FString
SerializeArray(const TArray<TSharedPtr<FJsonValue>> &Values) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&Json);
  check(FJsonSerializer::Serialize(Values, Writer));
  return Json;
}

} // namespace DataAdapters
