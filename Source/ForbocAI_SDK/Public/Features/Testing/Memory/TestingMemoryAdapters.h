#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/Fixture/FixtureAdapters.h"
#include "Features/Testing/Memory/TestingMemoryTypes.h"

namespace Testing::Memory {

/**
 * User Story: As a memory reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind<EMemoryTestActionKind>> & MemoryTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<EMemoryTestActionKind>> &
MemoryTestActionKinds() {
#define FORBOC_MEMORY_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), EMemoryTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<
      EMemoryTestActionKind>> Kinds = {
      FORBOC_MEMORY_TEST_ACTION_KIND(StoreStarted),
      FORBOC_MEMORY_TEST_ACTION_KIND(StoreSucceeded),
      FORBOC_MEMORY_TEST_ACTION_KIND(StoreFailed),
      FORBOC_MEMORY_TEST_ACTION_KIND(RecallStarted),
      FORBOC_MEMORY_TEST_ACTION_KIND(RecallSucceeded),
      FORBOC_MEMORY_TEST_ACTION_KIND(RecallFailed),
      FORBOC_MEMORY_TEST_ACTION_KIND(Cleared),
      FORBOC_MEMORY_TEST_ACTION_KIND(Inspect),
  };
#undef FORBOC_MEMORY_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(EMemoryTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing memory consumer, I need to invoke read memory test item through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryItem ReadMemoryTestItem(const TSharedPtr<FJsonObject> &Object) */
inline FMemoryItem
ReadMemoryTestItem(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return TypeFactory::MemoryItem(
      DataAdapters::ReadStringField(Value, TEXT("id")),
      DataAdapters::ReadStringField(Value, TEXT("text")),
      DataAdapters::ReadStringField(Value, TEXT("type")),
      DataAdapters::ReadFloatField(Value, TEXT("importance")),
      DataAdapters::ReadInt64Field(Value, TEXT("timestamp")));
}

/** User Story: As a features testing memory consumer, I need to invoke read memory test action through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryTestAction ReadMemoryTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FMemoryTestAction
ReadMemoryTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<EMemoryTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          MemoryTestActionKinds()),
      func::map_array<TSharedPtr<FJsonObject>, FMemoryItem>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("items")),
          ReadMemoryTestItem),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("targetId")),
  };
}

/** User Story: As a features testing memory consumer, I need to invoke read memory test expected through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryTestExpected ReadMemoryTestExpected(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing memory consumer, I need to invoke read memory test step through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryTestStep ReadMemoryTestStep(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing memory consumer, I need to invoke read memory test scenario through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryTestScenario ReadMemoryTestScenario(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing memory consumer, I need to invoke read memory test scenarios through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline TArray<FMemoryTestScenario> ReadMemoryTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FMemoryTestScenario>
ReadMemoryTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FMemoryTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadMemoryTestScenario);
}

/** User Story: As a features testing memory consumer, I need to invoke read memory test labels through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline FMemoryTestLabels ReadMemoryTestLabels(const DataAdapters::FSettingsSource &Source) */
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

/** User Story: As a features testing memory consumer, I need to invoke testing memory fixtures through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline const FMemoryTestFixtures &TestingMemoryFixtures() */
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

/** User Story: As a semantic-memory verifier, I need the vector contract loaded from authored data so TS and UE exercise the same positive and negative retrieval boundaries. @fn inline const FMemoryVectorTestFixtures &TestingMemoryVectorFixtures() */
inline const FMemoryVectorTestFixtures &TestingMemoryVectorFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/memory/vector.json"));
  static const FMemoryVectorTestFixtures Fixtures = []() {
    const TSharedRef<FJsonObject> Stories =
        DataAdapters::ReadObjectField(Source, TEXT("stories"));
    return FMemoryVectorTestFixtures{
        DataAdapters::ReadStringField(Source, TEXT("suite")),
        {DataAdapters::ReadStringField(Stories, TEXT("deterministic")),
         DataAdapters::ReadStringField(Stories, TEXT("normalized")),
         DataAdapters::ReadStringField(Stories, TEXT("naturalQuestion")),
         DataAdapters::ReadStringField(Stories, TEXT("unrelatedQuestion"))},
        DataAdapters::ReadStringField(Source, TEXT("fact")),
        DataAdapters::ReadStringField(Source, TEXT("naturalQuestion")),
        DataAdapters::ReadStringField(Source, TEXT("unrelatedQuestion")),
        DataAdapters::ReadFloatField(Source,
                                     TEXT("minimumRelevantSimilarity")),
        DataAdapters::ReadFloatField(Source,
                                     TEXT("maximumUnrelatedSimilarity")),
        DataAdapters::ReadFloatField(Source, TEXT("unitNorm")),
        DataAdapters::ReadFloatField(Source, TEXT("precision"))};
  }();
  return Fixtures;
}

/** User Story: As a persistent-memory release verifier, I need migration records, versions, scenarios, and assertions loaded from authored data so the native contract test contains no hidden fixture behavior. @fn inline const FMemoryContractTestFixtures &TestingMemoryContractFixtures() */
inline const FMemoryContractTestFixtures &TestingMemoryContractFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/memory/contract.json"));
  static const FMemoryContractTestFixtures Fixtures = []() {
    const TSharedRef<FJsonObject> Record =
        DataAdapters::ReadObjectField(Source, TEXT("record"));
    const TSharedRef<FJsonObject> Numbers =
        DataAdapters::ReadObjectField(Source, TEXT("numbers"));
    const TSharedRef<FJsonObject> Scenarios =
        DataAdapters::ReadObjectField(Source, TEXT("scenarios"));
    const TSharedRef<FJsonObject> Assertions =
        DataAdapters::ReadObjectField(Source, TEXT("assertions"));
    return FMemoryContractTestFixtures{
        DataAdapters::ReadStringField(Source, TEXT("suite")),
        DataAdapters::ReadStringField(Source, TEXT("databasePrefix")),
        {DataAdapters::ReadStringField(Record, TEXT("id")),
         DataAdapters::ReadStringField(Record, TEXT("text")),
         DataAdapters::ReadStringField(Record, TEXT("type")),
         DataAdapters::ReadFloatField(Record, TEXT("importance")),
         DataAdapters::ReadInt64Field(Record, TEXT("timestamp"))},
        {DataAdapters::ReadFloatField(Numbers, TEXT("legacyVectorValue")),
         DataAdapters::ReadFloatField(Numbers,
                                      TEXT("legacyFirstVectorValue")),
         DataAdapters::ReadNumberField(Numbers, TEXT("expectedCount")),
         DataAdapters::ReadNumberField(Numbers, TEXT("firstIndex")),
         DataAdapters::ReadNumberField(Numbers,
                                       TEXT("similarityPrecision"))},
        {DataAdapters::ReadStringField(Scenarios, TEXT("legacyMigration")),
         DataAdapters::ReadStringField(Scenarios, TEXT("newerRejection"))},
        {DataAdapters::ReadStringField(Assertions, TEXT("legacyCreated")),
         DataAdapters::ReadStringField(Assertions, TEXT("legacyOpened")),
         DataAdapters::ReadStringField(Assertions, TEXT("migratedCount")),
         DataAdapters::ReadStringField(Assertions, TEXT("migratedText")),
         DataAdapters::ReadStringField(Assertions,
                                       TEXT("migratedSimilarity")),
         DataAdapters::ReadStringField(Assertions, TEXT("migratedVersion")),
         DataAdapters::ReadStringField(Assertions, TEXT("newerCreated")),
         DataAdapters::ReadStringField(Assertions, TEXT("newerRejected"))}};
  }();
  return Fixtures;
}

/** User Story: As a features testing memory consumer, I need to invoke find memory test scenario through a stable signature so the features testing memory workflow remains explicit and composable. @fn inline func::Maybe<FMemoryTestScenario> FindMemoryTestScenario(const FString &Name) */
inline func::Maybe<FMemoryTestScenario>
FindMemoryTestScenario(const FString &Name) {
  return func::find_array<FMemoryTestScenario>(
      TestingMemoryFixtures().Scenarios,
      [&Name](const FMemoryTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Memory
