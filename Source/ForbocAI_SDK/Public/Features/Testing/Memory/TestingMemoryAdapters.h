#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Memory/TestingMemoryTypes.h"

namespace Testing::Memory {

inline FMemoryItem
ReadMemoryTestItem(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return TypeFactory::MemoryItem(
      DataAdapters::ReadStringField(Value, TEXT("id")),
      DataAdapters::ReadStringField(Value, TEXT("text")),
      DataAdapters::ReadStringField(Value, TEXT("type")),
      DataAdapters::ReadFloatField(Value, TEXT("importance")),
      DataAdapters::ReadNumberField(Value, TEXT("timestamp")));
}

inline FMemoryTestAction
ReadMemoryTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(EMemoryTestActionKind::StoreStarted));
  check(Kind < static_cast<int32>(EMemoryTestActionKind::Count));
  return {
      static_cast<EMemoryTestActionKind>(Kind),
      func::map_array<TSharedPtr<FJsonObject>, FMemoryItem>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("items")),
          ReadMemoryTestItem),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("targetId")),
  };
}

inline FMemoryTestExpected
ReadMemoryTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("storageStatus")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("recallStatus")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("memoryCount")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("recalledCount")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("firstRecalledId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("selectedText")),
  };
}

inline FMemoryTestStep
ReadMemoryTestStep(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      ReadMemoryTestAction(
          DataAdapters::ReadObjectField(Value, TEXT("action"))),
      ReadMemoryTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

inline FMemoryTestScenario
ReadMemoryTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FMemoryTestStep>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("steps")),
          ReadMemoryTestStep),
  };
}

inline TArray<FMemoryTestScenario>
ReadMemoryTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FMemoryTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadMemoryTestScenario);
}

inline FMemoryTestLabels
ReadMemoryTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("suite")),
      DataAdapters::ReadStringField(Labels, TEXT("caseName")),
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("storageStatus")),
      DataAdapters::ReadStringField(Labels, TEXT("recallStatus")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
      DataAdapters::ReadStringField(Labels, TEXT("memoryCount")),
      DataAdapters::ReadStringField(Labels, TEXT("recalledCount")),
      DataAdapters::ReadStringField(Labels, TEXT("firstRecalledId")),
      DataAdapters::ReadStringField(Labels, TEXT("selectedText")),
  };
}

inline const FMemoryTestFixtures &TestingMemoryFixtures() {
  static const DataAdapters::FSettingsSource Settings =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/memory/settings.json"));
  static const TArray<DataAdapters::FArraySource> Catalogs = {
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/memory/scenarios/lifecycle.json")),
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/memory/scenarios/resilience.json")),
  };
  static const FMemoryTestFixtures Fixtures = {
      ReadMemoryTestLabels(Settings),
      func::concat_arrays<FMemoryTestScenario>(
          func::map_array<DataAdapters::FArraySource,
                          TArray<FMemoryTestScenario>>(
              Catalogs, ReadMemoryTestScenarios)),
  };
  return Fixtures;
}

inline func::Maybe<FMemoryTestScenario>
FindMemoryTestScenario(const FString &Name) {
  return func::find_array<FMemoryTestScenario>(
      TestingMemoryFixtures().Scenarios,
      [&Name](const FMemoryTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Memory
