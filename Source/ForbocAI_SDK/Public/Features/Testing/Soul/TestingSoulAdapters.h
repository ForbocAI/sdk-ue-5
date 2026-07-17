#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Fixture/FixtureAdapters.h"
#include "Features/Testing/Soul/Storage/Serialization/TestingSoulStorageSerializationAdapters.h"
#include "Features/Testing/Soul/TestingSoulTypes.h"

namespace Testing::Soul {

/**
 * User Story: As a Soul reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind<ESoulTestActionKind>> & SoulTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<ESoulTestActionKind>> &
SoulTestActionKinds() {
#define FORBOC_SOUL_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), ESoulTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<ESoulTestActionKind>>
      Kinds = {
          FORBOC_SOUL_TEST_ACTION_KIND(ExportPending),
          FORBOC_SOUL_TEST_ACTION_KIND(ExportSuccess),
          FORBOC_SOUL_TEST_ACTION_KIND(ExportFailed),
          FORBOC_SOUL_TEST_ACTION_KIND(ImportPending),
          FORBOC_SOUL_TEST_ACTION_KIND(ImportSuccess),
          FORBOC_SOUL_TEST_ACTION_KIND(ImportFailed),
          FORBOC_SOUL_TEST_ACTION_KIND(SetList),
          FORBOC_SOUL_TEST_ACTION_KIND(Clear),
          FORBOC_SOUL_TEST_ACTION_KIND(Inspect),
      };
#undef FORBOC_SOUL_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(ESoulTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing soul consumer, I need to invoke read soul test action through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline FSoulTestAction ReadSoulTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FSoulTestAction
ReadSoulTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<ESoulTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          SoulTestActionKinds()),
      DataAdapters::ReadOptionalStringField(Value, TEXT("txId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("soulId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("persona")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadStringArrayField(Value, TEXT("txIds")),
  };
}

/** User Story: As a features testing soul consumer, I need to invoke read soul test expected through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline FSoulTestExpected ReadSoulTestExpected(const TSharedPtr<FJsonObject> &Object) */
inline FSoulTestExpected
ReadSoulTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("exportStatus")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("importStatus")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasExportResult")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("exportResultTxId")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasImportedSoul")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("importedSoulId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("importedSoulPersona")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("availableSoulCount")),
      DataAdapters::ReadOptionalStringField(
          Value, TEXT("availableSoulFirstTxId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
  };
}

/** User Story: As a features testing soul consumer, I need to invoke read soul test step through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline FSoulTestStep ReadSoulTestStep( const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing soul consumer, I need to invoke read soul test scenario through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline FSoulTestScenario ReadSoulTestScenario(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing soul consumer, I need to invoke read soul test scenarios through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline TArray<FSoulTestScenario> ReadSoulTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FSoulTestScenario>
ReadSoulTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FSoulTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadSoulTestScenario);
}

/** User Story: As a features testing soul consumer, I need to invoke read soul test labels through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline FSoulTestLabels ReadSoulTestLabels(const DataAdapters::FSettingsSource &Source) */
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
      DataAdapters::ReadStringField(Labels, TEXT("hasExportResult")),
      DataAdapters::ReadStringField(Labels, TEXT("exportResultTxId")),
      DataAdapters::ReadStringField(Labels, TEXT("hasImportedSoul")),
      DataAdapters::ReadStringField(Labels, TEXT("importedSoulId")),
      DataAdapters::ReadStringField(Labels, TEXT("importedSoulPersona")),
      DataAdapters::ReadStringField(Labels, TEXT("availableSoulCount")),
      DataAdapters::ReadStringField(Labels, TEXT("availableSoulFirstTxId")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
      DataAdapters::ReadStringField(Labels, TEXT("providerRetryWithinCycle")),
      DataAdapters::ReadStringField(Labels, TEXT("providerRetryCycleBoundary")),
      DataAdapters::ReadStringField(
          Labels, TEXT("providerRetryBeforeMaximumCycle")),
      DataAdapters::ReadStringField(Labels,
                                    TEXT("providerRetryAtMaximumCycle")),
      DataAdapters::ReadStringField(Labels,
                                    TEXT("providerRetryWithoutGateways")),
      DataAdapters::ReadStringField(Labels, TEXT("storageWalletRoundTrip")),
      DataAdapters::ReadStringField(Labels, TEXT("storageEnvelopeRoundTrip")),
      DataAdapters::ReadStringField(Labels, TEXT("storageCatalogRoundTrip")),
  };
}

/**
 * User Story: As a Soul provider test, I need retry-cycle inputs decoded from authored fixtures.
 * @fn inline FSoulProviderRetryTestData ReadSoulProviderRetryTestData( const DataAdapters::FSettingsSource &Source)
 */
inline FSoulProviderRetryTestData ReadSoulProviderRetryTestData(
    const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Retry =
      DataAdapters::ReadObjectField(Source, TEXT("providerRetry"));
  return {
      DataAdapters::ReadNumberField(Retry, TEXT("withinCycleAttempt")),
      DataAdapters::ReadNumberField(Retry, TEXT("cycleBoundaryAttempt")),
      DataAdapters::ReadNumberField(Retry, TEXT("urlCount")),
  };
}

/** User Story: As a features testing soul consumer, I need to invoke testing soul fixtures through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline const FSoulTestFixtures &TestingSoulFixtures() */
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
      ReadSoulProviderRetryTestData(SettingsSource),
      Storage::Serialization::readStorageSerializationFixtureAdapter(
          SettingsSource),
      func::concat_arrays<FSoulTestScenario>(
          TArray<TArray<FSoulTestScenario>>{
              ReadSoulTestScenarios(LifecycleSource),
              ReadSoulTestScenarios(ResilienceSource),
              ReadSoulTestScenarios(ConcurrencySource),
          }),
  };
  return Fixtures;
}

/** User Story: As a features testing soul consumer, I need to invoke find soul test scenario through a stable signature so the features testing soul workflow remains explicit and composable. @fn inline func::Maybe<FSoulTestScenario> FindSoulTestScenario(const FString &Name) */
inline func::Maybe<FSoulTestScenario>
FindSoulTestScenario(const FString &Name) {
  return func::find_array<FSoulTestScenario>(
      TestingSoulFixtures().Scenarios,
      [&Name](const FSoulTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Soul
