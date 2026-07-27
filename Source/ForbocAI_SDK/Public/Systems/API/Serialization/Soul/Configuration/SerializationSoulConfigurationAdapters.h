#pragma once

#include "Components/API/Serialization/Soul/Configuration/SerializationSoulConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace JsonInterop::SoulConfiguration {

/**
 * User Story: As a Soul codec, I need portable payload fields loaded from the
 * same authored storage schema as the TS SDK.
 * @fn inline FSoulSerializationData readSoulSerializationData()
 */
inline FSoulSerializationData readSoulSerializationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/soul/storage.json"));
  const TSharedRef<FJsonObject> Soul =
      DataAdapters::ReadObjectField(Source, TEXT("soul"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Soul, TEXT("fields"));
  const TSharedRef<FJsonObject> Text =
      DataAdapters::ReadObjectField(Source, TEXT("text"));
  return {DataAdapters::ReadStringField(Soul, TEXT("version")),
          {DataAdapters::ReadStringField(Fields, TEXT("id")),
           DataAdapters::ReadStringField(Fields, TEXT("version")),
           DataAdapters::ReadStringField(Fields, TEXT("name")),
           DataAdapters::ReadStringField(Fields, TEXT("structuredPersona")),
           DataAdapters::ReadStringField(Fields, TEXT("memories")),
           DataAdapters::ReadStringField(Fields, TEXT("state"))},
          DataAdapters::ReadStringField(Text, TEXT("invalidSoul")),
          DataAdapters::ReadStringField(Text,
                                        TEXT("unsupportedSoulVersion"))};
}

/**
 * User Story: As every Soul codec, I need one immutable local payload schema
 * per process so encrypted bytes remain deterministic.
 * @fn inline const FSoulSerializationData &soulSerializationData()
 */
inline const FSoulSerializationData &soulSerializationData() {
  static const FSoulSerializationData Data = readSoulSerializationData();
  return Data;
}

} // namespace JsonInterop::SoulConfiguration
