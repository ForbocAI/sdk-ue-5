#pragma once

#include "Components/API/Endpoints/NPC/Generate/Configuration/GenerateConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace APISlice::Endpoints::NPCGenerateConfiguration {

/** User Story: As an NPC attribute generation codec, I need field names and the malformed-response error decoded from authored data. @fn inline FGenerateConfigurationData readGenerateConfigurationData() */
inline FGenerateConfigurationData readGenerateConfigurationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/npc-generate.json"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Errors =
      DataAdapters::ReadObjectField(Source, TEXT("errors"));
  FGenerateConfigurationData Data;
  Data.Fields.Attribute =
      DataAdapters::ReadStringField(Fields, TEXT("attribute"));
  Data.Fields.Context =
      DataAdapters::ReadStringField(Fields, TEXT("context"));
  Data.Fields.Value = DataAdapters::ReadStringField(Fields, TEXT("value"));
  Data.Errors.InvalidResponse =
      DataAdapters::ReadStringField(Errors, TEXT("invalidResponse"));
  return Data;
}

/** User Story: As an NPC attribute generation codec, I need the authored configuration memoized so field lookups stay cheap. @fn inline const FGenerateConfigurationData &generateConfigurationData() */
inline const FGenerateConfigurationData &generateConfigurationData() {
  static const FGenerateConfigurationData Data = readGenerateConfigurationData();
  return Data;
}

} // namespace APISlice::Endpoints::NPCGenerateConfiguration
