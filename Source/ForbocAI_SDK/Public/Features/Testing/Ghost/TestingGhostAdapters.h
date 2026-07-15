#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Ghost/TestingGhostTypes.h"

namespace Testing::Ghost {

inline FGhostTestAction
ReadGhostTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(EGhostTestActionKind::Start));
  check(Kind < static_cast<int32>(EGhostTestActionKind::Count));
  return {
      static_cast<EGhostTestActionKind>(Kind),
      DataAdapters::ReadOptionalStringField(Value, TEXT("sessionId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("status")),
      DataAdapters::ReadOptionalFloatField(Value, TEXT("progress")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadStringArrayField(Value, TEXT("resultScenarios")),
      DataAdapters::ReadStringArrayField(Value, TEXT("historySessionIds")),
  };
}

inline FGhostTestExpected
ReadGhostTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("activeSessionId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("status")),
      DataAdapters::ReadOptionalFloatField(Value, TEXT("progress")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasResults")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("resultCount")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("historyCount")),
      DataAdapters::ReadOptionalStringField(
          Value, TEXT("historyFirstSessionId")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("loading")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
  };
}

inline FGhostTestStep ReadGhostTestStep(
    const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      ReadGhostTestAction(
          DataAdapters::ReadObjectField(Value, TEXT("action"))),
      ReadGhostTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

inline FGhostTestScenario
ReadGhostTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FGhostTestStep>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("steps")),
          ReadGhostTestStep),
  };
}

inline TArray<FGhostTestScenario>
ReadGhostTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FGhostTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadGhostTestScenario);
}

inline FGhostTestLabels
ReadGhostTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("suite")),
      DataAdapters::ReadStringField(Labels, TEXT("caseName")),
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("activeSessionId")),
      DataAdapters::ReadStringField(Labels, TEXT("status")),
      DataAdapters::ReadStringField(Labels, TEXT("progress")),
      DataAdapters::ReadStringField(Labels, TEXT("hasResults")),
      DataAdapters::ReadStringField(Labels, TEXT("resultCount")),
      DataAdapters::ReadStringField(Labels, TEXT("historyCount")),
      DataAdapters::ReadStringField(Labels, TEXT("historyFirstSessionId")),
      DataAdapters::ReadStringField(Labels, TEXT("loading")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
  };
}

inline const FGhostTestFixtures &TestingGhostFixtures() {
  static const DataAdapters::FSettingsSource SettingsSource =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/ghost/settings.json"));
  static const DataAdapters::FArraySource LifecycleSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/ghost/scenarios/lifecycle.json"));
  static const DataAdapters::FArraySource ResilienceSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/ghost/scenarios/resilience.json"));
  static const FGhostTestFixtures Fixtures = {
      ReadGhostTestLabels(SettingsSource),
      func::concat_arrays<FGhostTestScenario>(
          TArray<TArray<FGhostTestScenario>>{
              ReadGhostTestScenarios(LifecycleSource),
              ReadGhostTestScenarios(ResilienceSource),
          }),
  };
  return Fixtures;
}

inline func::Maybe<FGhostTestScenario>
FindGhostTestScenario(const FString &Name) {
  return func::find_array<FGhostTestScenario>(
      TestingGhostFixtures().Scenarios,
      [&Name](const FGhostTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Ghost
