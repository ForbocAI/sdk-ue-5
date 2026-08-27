#pragma once

#include "Components/Testing/Config/TestingConfigTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace TestingConfigAdapters {

/** User Story: As Config regression tests, I need endpoint fixtures and assertion labels decoded from authored JSON. @fn inline FTestingConfigData readTestingConfigData() */
inline FTestingConfigData readTestingConfigData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/tests/config/runtime.json"));
  const TSharedRef<FJsonObject> Values =
      DataAdapters::ReadObjectField(Source, TEXT("values"));
  const TSharedRef<FJsonObject> Counts =
      DataAdapters::ReadObjectField(Source, TEXT("counts"));
  const TSharedRef<FJsonObject> Assertions =
      DataAdapters::ReadObjectField(Source, TEXT("assertions"));
  return {
      DataAdapters::ReadStringField(Source, TEXT("automationName")),
      {DataAdapters::ReadStringField(Values, TEXT("empty")),
       DataAdapters::ReadStringField(Values, TEXT("explicitApiUrl")),
       DataAdapters::ReadStringField(Values,
                                     TEXT("normalizedExplicitApiUrl")),
       DataAdapters::ReadStringField(Values, TEXT("apiKey")),
       DataAdapters::ReadStringField(Values, TEXT("missingConfigPath")),
       DataAdapters::ReadBooleanField(Values, TEXT("localAvailable")),
       DataAdapters::ReadBooleanField(Values, TEXT("localUnavailable"))},
      {DataAdapters::ReadNumberField(Counts, TEXT("none")),
       DataAdapters::ReadNumberField(Counts, TEXT("increment")),
       DataAdapters::ReadNumberField(Counts, TEXT("once"))},
      {DataAdapters::ReadStringField(Assertions, TEXT("explicitUrl")),
       DataAdapters::ReadStringField(Assertions, TEXT("explicitSource")),
       DataAdapters::ReadStringField(Assertions, TEXT("explicitProbe")),
       DataAdapters::ReadStringField(Assertions, TEXT("localUrl")),
       DataAdapters::ReadStringField(Assertions, TEXT("localSource")),
       DataAdapters::ReadStringField(Assertions, TEXT("localProbe")),
       DataAdapters::ReadStringField(Assertions, TEXT("productionUrl")),
       DataAdapters::ReadStringField(Assertions, TEXT("productionSource")),
       DataAdapters::ReadStringField(Assertions, TEXT("timeoutProbe")),
       DataAdapters::ReadStringField(Assertions, TEXT("productionKey")),
       DataAdapters::ReadStringField(Assertions, TEXT("localKey")),
       DataAdapters::ReadStringField(Assertions, TEXT("redactedUrl")),
       DataAdapters::ReadStringField(Assertions, TEXT("redactedKey"))}};
}

/** User Story: As Config regression tests, I need one immutable authored fixture shared by registration and assertions. @fn inline const FTestingConfigData &testingConfigData() */
inline const FTestingConfigData &testingConfigData() {
  static const FTestingConfigData Data = readTestingConfigData();
  return Data;
}

} // namespace TestingConfigAdapters
