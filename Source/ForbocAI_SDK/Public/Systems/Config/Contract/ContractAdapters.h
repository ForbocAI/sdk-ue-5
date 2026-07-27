#pragma once

#include "Components/Config/ConfigTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ConfigSlice {

/**
 * @fn inline FConfigRuntimeData readConfigRuntimeData()
 * User Story: As configuration ownership, I need the shared authored contract decoded once so TS and UE use the same defaults, keys, and persistence policy.
 */
inline FConfigRuntimeData readConfigRuntimeData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/config/runtime.json"));
  const TSharedRef<FJsonObject> Defaults =
      DataAdapters::ReadObjectField(Source, TEXT("defaults"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Slice =
      DataAdapters::ReadObjectField(Source, TEXT("slice"));
  const TSharedRef<FJsonObject> Actions =
      DataAdapters::ReadObjectField(Slice, TEXT("actions"));
  const TSharedRef<FJsonObject> Environment =
      DataAdapters::ReadObjectField(Source, TEXT("environment"));
  const TSharedRef<FJsonObject> Path =
      DataAdapters::ReadObjectField(Source, TEXT("path"));
  const TSharedRef<FJsonObject> Serialization =
      DataAdapters::ReadObjectField(Source, TEXT("serialization"));
  const TSharedRef<FJsonObject> Errors =
      DataAdapters::ReadObjectField(Source, TEXT("errors"));
  return {{DataAdapters::ReadStringField(Defaults, TEXT("sdkVersion")),
           DataAdapters::ReadStringField(Defaults, TEXT("apiUrl")),
           DataAdapters::ReadStringField(Defaults, TEXT("apiKey")),
           DataAdapters::ReadStringField(Defaults, TEXT("databasePath")),
           DataAdapters::ReadNumberField(Defaults, TEXT("vectorDimension")),
           DataAdapters::ReadNumberField(Defaults, TEXT("maxRecallResults"))},
          {DataAdapters::ReadStringField(Fields, TEXT("sdkVersion")),
           DataAdapters::ReadStringField(Fields, TEXT("apiUrl")),
           DataAdapters::ReadStringField(Fields, TEXT("apiKey")),
           DataAdapters::ReadStringField(Fields, TEXT("databasePath")),
           DataAdapters::ReadStringField(Fields, TEXT("vectorDimension")),
           DataAdapters::ReadStringField(Fields, TEXT("maxRecallResults"))},
          {DataAdapters::ReadStringField(Slice, TEXT("name")),
           DataAdapters::ReadStringField(Slice, TEXT("actionSeparator")),
           {DataAdapters::ReadStringField(Actions, TEXT("hydrated")),
            DataAdapters::ReadStringField(Actions, TEXT("entryCommitted")),
            DataAdapters::ReadStringField(Actions, TEXT("apiCommitted"))}},
          {DataAdapters::ReadStringField(Environment, TEXT("homeKey")),
           DataAdapters::ReadStringField(Environment, TEXT("profileKey")),
           DataAdapters::ReadStringField(Environment, TEXT("apiUrlKey")),
           DataAdapters::ReadStringField(Environment, TEXT("apiKeyKey")),
           DataAdapters::ReadStringField(Environment, TEXT("databasePathKey")),
           DataAdapters::ReadStringField(Environment, TEXT("vectorDimensionKey")),
           DataAdapters::ReadStringField(Environment, TEXT("maxRecallResultsKey"))},
          {DataAdapters::ReadStringField(Path, TEXT("homeFallback")),
           DataAdapters::ReadStringField(Path, TEXT("fileName"))},
          {DataAdapters::ReadStringField(Serialization, TEXT("encoding")),
           DataAdapters::ReadNumberField(Serialization, TEXT("indentation")),
           DataAdapters::ReadNumberField(Serialization, TEXT("fileMode"))},
          {DataAdapters::ReadStringField(Errors, TEXT("invalidRoot"))}};
}

/**
 * @fn inline const FConfigRuntimeData &configRuntimeData()
 * User Story: As configuration consumers, I need one immutable decoded contract shared by adapters, actions, reducers, and selectors.
 */
inline const FConfigRuntimeData &configRuntimeData() {
  static const FConfigRuntimeData Data = readConfigRuntimeData();
  return Data;
}

} // namespace ConfigSlice
