#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Fixture/FixtureAdapters.h"
#include "Features/Testing/Ghost/TestingGhostTypes.h"

namespace Testing::Ghost {

/**
 * User Story: As a ghost reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind<EGhostTestActionKind>> & GhostTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<EGhostTestActionKind>> &
GhostTestActionKinds() {
#define FORBOC_GHOST_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), EGhostTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<
      EGhostTestActionKind>> Kinds = {
      FORBOC_GHOST_TEST_ACTION_KIND(Start),
      FORBOC_GHOST_TEST_ACTION_KIND(Progress),
      FORBOC_GHOST_TEST_ACTION_KIND(Complete),
      FORBOC_GHOST_TEST_ACTION_KIND(Fail),
      FORBOC_GHOST_TEST_ACTION_KIND(LoadHistory),
      FORBOC_GHOST_TEST_ACTION_KIND(Clear),
      FORBOC_GHOST_TEST_ACTION_KIND(Inspect),
  };
#undef FORBOC_GHOST_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(EGhostTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test action through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline FGhostTestAction ReadGhostTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FGhostTestAction
ReadGhostTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<EGhostTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          GhostTestActionKinds()),
      DataAdapters::ReadOptionalStringField(Value, TEXT("sessionId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("status")),
      DataAdapters::ReadOptionalFloatField(Value, TEXT("progress")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadStringArrayField(Value, TEXT("resultScenarios")),
      DataAdapters::ReadStringArrayField(Value, TEXT("historySessionIds")),
  };
}

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test expected through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline FGhostTestExpected ReadGhostTestExpected(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test step through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline FGhostTestStep ReadGhostTestStep( const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test scenario through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline FGhostTestScenario ReadGhostTestScenario(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test scenarios through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline TArray<FGhostTestScenario> ReadGhostTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FGhostTestScenario>
ReadGhostTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FGhostTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadGhostTestScenario);
}

/** User Story: As a features testing ghost consumer, I need to invoke read ghost test labels through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline FGhostTestLabels ReadGhostTestLabels(const DataAdapters::FSettingsSource &Source) */
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

/** User Story: As a features testing ghost consumer, I need to invoke testing ghost fixtures through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline const FGhostTestFixtures &TestingGhostFixtures() */
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

/** User Story: As a features testing ghost consumer, I need to invoke find ghost test scenario through a stable signature so the features testing ghost workflow remains explicit and composable. @fn inline func::Maybe<FGhostTestScenario> FindGhostTestScenario(const FString &Name) */
inline func::Maybe<FGhostTestScenario>
FindGhostTestScenario(const FString &Name) {
  return func::find_array<FGhostTestScenario>(
      TestingGhostFixtures().Scenarios,
      [&Name](const FGhostTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Ghost
