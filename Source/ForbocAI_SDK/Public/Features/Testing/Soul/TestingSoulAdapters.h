#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Soul/TestingSoulTypes.h"

namespace Testing::Soul {

inline FSoulTestAction
ReadSoulTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(ESoulTestActionKind::ExportPending));
  check(Kind < static_cast<int32>(ESoulTestActionKind::Count));
  return {
      static_cast<ESoulTestActionKind>(Kind),
      DataAdapters::ReadOptionalStringField(Value, TEXT("txId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("soulId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("persona")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadStringArrayField(Value, TEXT("txIds")),
  };
}

inline FSoulTestExpected
ReadSoulTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("exportStatus")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("importStatus")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasLastExport")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("lastExportTxId")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasLastImport")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("lastImportId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("lastImportPersona")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("availableSoulCount")),
      DataAdapters::ReadOptionalStringField(
          Value, TEXT("availableSoulFirstTxId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
  };
}

inline FSoulTestStep ReadSoulTestStep(
    const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      ReadSoulTestAction(DataAdapters::ReadObjectField(Value, TEXT("action"))),
      ReadSoulTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

inline FSoulTestScenario
ReadSoulTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FSoulTestStep>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("steps")),
          ReadSoulTestStep),
  };
}

inline TArray<FSoulTestScenario>
ReadSoulTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FSoulTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadSoulTestScenario);
}

inline FSoulTestLabels
ReadSoulTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("suite")),
      DataAdapters::ReadStringField(Labels, TEXT("caseName")),
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("exportStatus")),
      DataAdapters::ReadStringField(Labels, TEXT("importStatus")),
      DataAdapters::ReadStringField(Labels, TEXT("hasLastExport")),
      DataAdapters::ReadStringField(Labels, TEXT("lastExportTxId")),
      DataAdapters::ReadStringField(Labels, TEXT("hasLastImport")),
      DataAdapters::ReadStringField(Labels, TEXT("lastImportId")),
      DataAdapters::ReadStringField(Labels, TEXT("lastImportPersona")),
      DataAdapters::ReadStringField(Labels, TEXT("availableSoulCount")),
      DataAdapters::ReadStringField(Labels, TEXT("availableSoulFirstTxId")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
  };
}

inline const FSoulTestFixtures &TestingSoulFixtures() {
  static const DataAdapters::FSettingsSource SettingsSource =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/soul/settings.json"));
  static const DataAdapters::FArraySource LifecycleSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/soul/scenarios/lifecycle.json"));
  static const DataAdapters::FArraySource ResilienceSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/soul/scenarios/resilience.json"));
  static const DataAdapters::FArraySource ConcurrencySource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/soul/scenarios/concurrency.json"));
  static const FSoulTestFixtures Fixtures = {
      ReadSoulTestLabels(SettingsSource),
      func::concat_arrays<FSoulTestScenario>(
          TArray<TArray<FSoulTestScenario>>{
              ReadSoulTestScenarios(LifecycleSource),
              ReadSoulTestScenarios(ResilienceSource),
              ReadSoulTestScenarios(ConcurrencySource),
          }),
  };
  return Fixtures;
}

inline func::Maybe<FSoulTestScenario>
FindSoulTestScenario(const FString &Name) {
  return func::find_array<FSoulTestScenario>(
      TestingSoulFixtures().Scenarios,
      [&Name](const FSoulTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Soul
