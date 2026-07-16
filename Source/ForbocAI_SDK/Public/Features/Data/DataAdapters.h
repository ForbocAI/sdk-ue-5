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

/**
 * @fn inline FString ModuleContentDir(const FString &ModuleName)
 * User Story: As an SDK consumer, I need authored data resolved from the module's active Unreal host so the same SDK works as a packaged plugin and as its CLI project module.
 */
inline FString ModuleContentDir(const FString &ModuleName) {
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(ModuleName);
  return Plugin.IsValid() ? Plugin->GetContentDir()
                          : FPaths::ProjectContentDir();
}

/** User Story: As a features data consumer, I need to invoke settings source through a stable signature so the features data workflow remains explicit and composable. @fn inline FSettingsSource SettingsSource(const FString &PluginName, const FString &RelativePath) */
inline FSettingsSource SettingsSource(const FString &PluginName,
                                      const FString &RelativePath) {
  const FString Path =
      FPaths::Combine(ModuleContentDir(PluginName), RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TSharedPtr<FJsonObject> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
  check(FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid());
  return {Root.ToSharedRef()};
}

/** User Story: As a features data consumer, I need to invoke array source through a stable signature so the features data workflow remains explicit and composable. @fn inline FArraySource ArraySource(const FString &PluginName, const FString &RelativePath) */
inline FArraySource ArraySource(const FString &PluginName,
                                const FString &RelativePath) {
  const FString Path =
      FPaths::Combine(ModuleContentDir(PluginName), RelativePath);
  FString Json;
  check(FFileHelper::LoadFileToString(Json, *Path));
  TArray<TSharedPtr<FJsonValue>> Root;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
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

/** User Story: As a features data consumer, I need to invoke read string field through a stable signature so the features data workflow remains explicit and composable. @fn inline FString ReadStringField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline FString ReadStringField(const TSharedRef<FJsonObject> &Object,
                               const FString &Field) {
  return Object->GetStringField(Field);
}

/** User Story: As a features data consumer, I need to invoke read optional string field through a stable signature so the features data workflow remains explicit and composable. @fn inline func::Maybe<FString> ReadOptionalStringField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline func::Maybe<FString>
ReadOptionalStringField(const TSharedRef<FJsonObject> &Object,
                        const FString &Field) {
  FString Value;
  return Object->TryGetStringField(Field, Value)
             ? func::just<FString>(Value)
             : func::nothing<FString>();
}

/** User Story: As a features data consumer, I need to invoke read number field through a stable signature so the features data workflow remains explicit and composable. @fn inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline int32 ReadNumberField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return static_cast<int32>(Object->GetNumberField(Field));
}

/** User Story: As authored data parsing, I need unsigned integer fields preserved without signed overflow so portable hash and bitmask configuration remains exact. @fn inline uint32 ReadUInt32Field(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline uint32 ReadUInt32Field(const TSharedRef<FJsonObject> &Object,
                              const FString &Field) {
  return static_cast<uint32>(Object->GetNumberField(Field));
}

/** User Story: As a features data consumer, I need to invoke read optional number field through a stable signature so the features data workflow remains explicit and composable. @fn inline func::Maybe<int32> ReadOptionalNumberField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline func::Maybe<int32>
ReadOptionalNumberField(const TSharedRef<FJsonObject> &Object,
                        const FString &Field) {
  double Value;
  return Object->TryGetNumberField(Field, Value)
             ? func::just<int32>(static_cast<int32>(Value))
             : func::nothing<int32>();
}

/** User Story: As a features data consumer, I need to invoke read float field through a stable signature so the features data workflow remains explicit and composable. @fn inline float ReadFloatField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline float ReadFloatField(const TSharedRef<FJsonObject> &Object,
                            const FString &Field) {
  return static_cast<float>(Object->GetNumberField(Field));
}

/** User Story: As a features data consumer, I need to invoke read optional float field through a stable signature so the features data workflow remains explicit and composable. @fn inline func::Maybe<float> ReadOptionalFloatField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline func::Maybe<float>
ReadOptionalFloatField(const TSharedRef<FJsonObject> &Object,
                       const FString &Field) {
  double Value;
  return Object->TryGetNumberField(Field, Value)
             ? func::just<float>(static_cast<float>(Value))
             : func::nothing<float>();
}

/** User Story: As a features data consumer, I need to invoke read boolean field through a stable signature so the features data workflow remains explicit and composable. @fn inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline bool ReadBooleanField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->GetBoolField(Field);
}

/** User Story: As a features data consumer, I need to invoke read optional boolean field through a stable signature so the features data workflow remains explicit and composable. @fn inline func::Maybe<bool> ReadOptionalBooleanField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline func::Maybe<bool>
ReadOptionalBooleanField(const TSharedRef<FJsonObject> &Object,
                         const FString &Field) {
  bool Value;
  return Object->TryGetBoolField(Field, Value) ? func::just<bool>(Value)
                                               : func::nothing<bool>();
}

/** User Story: As a features data consumer, I need to invoke read number array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<int32> ReadNumberArrayField( const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<int32> ReadNumberArrayField(
    const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, int32>(
      Object->GetArrayField(Field),
      [](const TSharedPtr<FJsonValue> &Value) {
        return static_cast<int32>(Value->AsNumber());
      });
}

/** User Story: As a features data consumer, I need to invoke read array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<TSharedPtr<FJsonValue>> ReadArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<TSharedPtr<FJsonValue>>
ReadArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) {
  return Object->GetArrayField(Field);
}

/** User Story: As a features data consumer, I need to invoke read string array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<FString> ReadStringArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<FString>
ReadStringArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, FString>(
      ReadArrayField(Object, Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsString(); });
}

/** User Story: As a features data consumer, I need to invoke read optional string array field through a stable signature so the features data workflow remains explicit and composable. @fn inline func::Maybe<TArray<FString>> ReadOptionalStringArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline func::Maybe<TArray<FString>>
ReadOptionalStringArrayField(const TSharedRef<FJsonObject> &Object,
                             const FString &Field) {
  return Object->HasField(Field)
             ? func::just<TArray<FString>>(ReadStringArrayField(Object, Field))
             : func::nothing<TArray<FString>>();
}

/** User Story: As a features data consumer, I need to invoke read object array field through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<TSharedPtr<FJsonObject>> ReadObjectArrayField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<TSharedPtr<FJsonObject>>
ReadObjectArrayField(const TSharedRef<FJsonObject> &Object,
                     const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, TSharedPtr<FJsonObject>>(
      ReadArrayField(Object, Field),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsObject(); });
}

/** User Story: As a features data consumer, I need to invoke read object array through a stable signature so the features data workflow remains explicit and composable. @fn inline TArray<TSharedPtr<FJsonObject>> ReadObjectArray(const FArraySource &Source) */
inline TArray<TSharedPtr<FJsonObject>>
ReadObjectArray(const FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonValue>, TSharedPtr<FJsonObject>>(
      Source.Root,
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsObject(); });
}

/** User Story: As a features data consumer, I need to invoke read number vector field through a stable signature so the features data workflow remains explicit and composable. @fn inline std::vector<int> ReadNumberVectorField(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline std::vector<int>
ReadNumberVectorField(const TSharedRef<FJsonObject> &Object,
                      const FString &Field) {
  const TArray<int32> Values = ReadNumberArrayField(Object, Field);
  return std::vector<int>(Values.begin(), Values.end());
}

/** User Story: As a features data consumer, I need to invoke serialize object through a stable signature so the features data workflow remains explicit and composable. @fn inline FString SerializeObject(const TSharedRef<FJsonObject> &Object) */
inline FString SerializeObject(const TSharedRef<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&Json);
  check(FJsonSerializer::Serialize(Object, Writer));
  return Json;
}

/** User Story: As a features data consumer, I need to invoke serialize array through a stable signature so the features data workflow remains explicit and composable. @fn inline FString SerializeArray(const TArray<TSharedPtr<FJsonValue>> &Values) */
inline FString
SerializeArray(const TArray<TSharedPtr<FJsonValue>> &Values) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&Json);
  check(FJsonSerializer::Serialize(Values, Writer));
  return Json;
}

} // namespace DataAdapters
