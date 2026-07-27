#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Systems/Testing/Fixture/FixtureAdapters.h"
#include "Components/Testing/Directive/TestingDirectiveTypes.h"

namespace Testing::Directive {

/**
 * User Story: As a directive reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind< EDirectiveTestActionKind>> & DirectiveTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<
    EDirectiveTestActionKind>> &
DirectiveTestActionKinds() {
#define FORBOC_DIRECTIVE_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), EDirectiveTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<
      EDirectiveTestActionKind>> Kinds = {
      FORBOC_DIRECTIVE_TEST_ACTION_KIND(Start),
      FORBOC_DIRECTIVE_TEST_ACTION_KIND(Receive),
      FORBOC_DIRECTIVE_TEST_ACTION_KIND(Validate),
      FORBOC_DIRECTIVE_TEST_ACTION_KIND(Fail),
      FORBOC_DIRECTIVE_TEST_ACTION_KIND(ClearNpc),
  };
#undef FORBOC_DIRECTIVE_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(EDirectiveTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test action through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline FDirectiveTestAction ReadDirectiveTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FDirectiveTestAction
ReadDirectiveTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<EDirectiveTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          DirectiveTestActionKinds()),
      DataAdapters::ReadOptionalStringField(Value, TEXT("id")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("npcId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("observation")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("query")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("limit")),
      DataAdapters::ReadOptionalFloatField(Value, TEXT("threshold")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("verdictValid")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("dialogue")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasAction")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("actionType")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("actionTarget")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("actionReason")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
  };
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test run expected through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline FDirectiveTestRunExpected ReadDirectiveTestRunExpected(const TSharedPtr<FJsonObject> &Object) */
inline FDirectiveTestRunExpected
ReadDirectiveTestRunExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("id")),
      DataAdapters::ReadBooleanField(Value, TEXT("exists")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("status")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("npcId")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("observation")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("recallQuery")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("recallLimit")),
      DataAdapters::ReadOptionalFloatField(Value, TEXT("recallThreshold")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("verdictValid")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("dialogue")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("actionType")),
  };
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test expected through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline FDirectiveTestExpected ReadDirectiveTestExpected(const TSharedRef<FJsonObject> &Object) */
inline FDirectiveTestExpected
ReadDirectiveTestExpected(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadNumberField(Object, TEXT("directiveCount")),
      DataAdapters::ReadStringField(Object, TEXT("activeId")),
      func::map_array<TSharedPtr<FJsonObject>, FDirectiveTestRunExpected>(
          DataAdapters::ReadObjectArrayField(Object, TEXT("runs")),
          ReadDirectiveTestRunExpected),
  };
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test scenario through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline FDirectiveTestScenario ReadDirectiveTestScenario(const TSharedPtr<FJsonObject> &Object) */
inline FDirectiveTestScenario
ReadDirectiveTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FDirectiveTestAction>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("actions")),
          ReadDirectiveTestAction),
      ReadDirectiveTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test scenarios through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline TArray<FDirectiveTestScenario> ReadDirectiveTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FDirectiveTestScenario>
ReadDirectiveTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>,
                         FDirectiveTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadDirectiveTestScenario);
}

/** User Story: As a features testing directive consumer, I need to invoke read directive test labels through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline FDirectiveTestLabels ReadDirectiveTestLabels(const DataAdapters::FSettingsSource &Source) */
inline FDirectiveTestLabels
ReadDirectiveTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("directiveCount")),
      DataAdapters::ReadStringField(Labels, TEXT("activeId")),
      DataAdapters::ReadStringField(Labels, TEXT("activePresence")),
      DataAdapters::ReadStringField(Labels, TEXT("runPresence")),
      DataAdapters::ReadStringField(Labels, TEXT("status")),
      DataAdapters::ReadStringField(Labels, TEXT("npcId")),
      DataAdapters::ReadStringField(Labels, TEXT("observation")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
      DataAdapters::ReadStringField(Labels, TEXT("recallQuery")),
      DataAdapters::ReadStringField(Labels, TEXT("recallLimit")),
      DataAdapters::ReadStringField(Labels, TEXT("recallThreshold")),
      DataAdapters::ReadStringField(Labels, TEXT("verdictValid")),
      DataAdapters::ReadStringField(Labels, TEXT("dialogue")),
      DataAdapters::ReadStringField(Labels, TEXT("actionType")),
  };
}

/** User Story: As a features testing directive consumer, I need to invoke directive test fixtures through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline const FDirectiveTestFixtures &DirectiveTestFixtures() */
inline const FDirectiveTestFixtures &DirectiveTestFixtures() {
  static const DataAdapters::FSettingsSource SettingsSource =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/directive/settings.json"));
  static const DataAdapters::FArraySource LifecycleSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/directive/scenarios/lifecycle.json"));
  static const DataAdapters::FArraySource ResilienceSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/directive/scenarios/resilience.json"));
  static const DataAdapters::FArraySource EdgeSource =
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/directive/scenarios/edge.json"));
  static const FDirectiveTestFixtures Fixtures = {
      ReadDirectiveTestLabels(SettingsSource),
      func::concat_arrays<FDirectiveTestScenario>(
          TArray<TArray<FDirectiveTestScenario>>{
              ReadDirectiveTestScenarios(LifecycleSource),
              ReadDirectiveTestScenarios(ResilienceSource),
              ReadDirectiveTestScenarios(EdgeSource),
          }),
  };
  return Fixtures;
}

/** User Story: As a features testing directive consumer, I need to invoke find directive test scenario through a stable signature so the features testing directive workflow remains explicit and composable. @fn inline func::Maybe<FDirectiveTestScenario> FindDirectiveTestScenario(const FString &Name) */
inline func::Maybe<FDirectiveTestScenario>
FindDirectiveTestScenario(const FString &Name) {
  return func::find_array<FDirectiveTestScenario>(
      DirectiveTestFixtures().Scenarios,
      [&Name](const FDirectiveTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Directive
