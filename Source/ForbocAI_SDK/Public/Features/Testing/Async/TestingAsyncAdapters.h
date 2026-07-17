#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/Async/TestingAsyncTypes.h"

namespace TestingAsyncAdapters {

/** User Story: As an Async regression test, I need test identity, values, and assertions loaded from authored data through a stable signature. @fn inline FTestingAsyncData readTestingAsyncData() */
inline FTestingAsyncData readTestingAsyncData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/tests/async/wait.json"));
  const TSharedRef<FJsonObject> Values =
      DataAdapters::ReadObjectField(Source, TEXT("values"));
  const TSharedRef<FJsonObject> Assertions =
      DataAdapters::ReadObjectField(Source, TEXT("assertions"));
  return {
      DataAdapters::ReadStringField(Source.Root, TEXT("automationName")),
      {DataAdapters::ReadNumberField(Values, TEXT("immediate")),
       DataAdapters::ReadNumberField(Values, TEXT("late")),
       DataAdapters::ReadFloatField(Values, TEXT("timeoutSeconds"))},
      {DataAdapters::ReadStringField(Assertions, TEXT("immediate")),
       DataAdapters::ReadStringField(Assertions, TEXT("timeout")),
       DataAdapters::ReadStringField(Assertions, TEXT("callback")),
       DataAdapters::ReadStringField(Assertions, TEXT("late"))}};
}

/** User Story: As an Async regression test, I need one immutable authored test document shared by registration and assertions. @fn inline const FTestingAsyncData &testingAsyncData() */
inline const FTestingAsyncData &testingAsyncData() {
  static const FTestingAsyncData Data = readTestingAsyncData();
  return Data;
}

} // namespace TestingAsyncAdapters
