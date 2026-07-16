#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Fixture/FixtureAdapters.h"
#include "Features/Testing/Bridge/TestingBridgeTypes.h"

namespace Testing::Bridge {

/**
 * User Story: As a bridge reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind<EBridgeTestActionKind>> & BridgeTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<EBridgeTestActionKind>> &
BridgeTestActionKinds() {
#define FORBOC_BRIDGE_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), EBridgeTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<
      EBridgeTestActionKind>> Kinds = {
      FORBOC_BRIDGE_TEST_ACTION_KIND(ValidationRequested),
      FORBOC_BRIDGE_TEST_ACTION_KIND(ValidationSucceeded),
      FORBOC_BRIDGE_TEST_ACTION_KIND(ValidationFailed),
      FORBOC_BRIDGE_TEST_ACTION_KIND(ActivePresetsReceived),
      FORBOC_BRIDGE_TEST_ACTION_KIND(ActivePresetAdded),
      FORBOC_BRIDGE_TEST_ACTION_KIND(RulesetsReceived),
      FORBOC_BRIDGE_TEST_ACTION_KIND(PresetIdsReceived),
      FORBOC_BRIDGE_TEST_ACTION_KIND(ValidationCleared),
      FORBOC_BRIDGE_TEST_ACTION_KIND(Inspect),
  };
#undef FORBOC_BRIDGE_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(EBridgeTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test ruleset through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FDirectiveRuleSet ReadBridgeTestRuleset(const TSharedPtr<FJsonObject> &Object) */
inline FDirectiveRuleSet
ReadBridgeTestRuleset(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FDirectiveRuleSet Ruleset;
  Ruleset.RulesetId =
      DataAdapters::ReadStringField(Value, TEXT("rulesetId"));
  return Ruleset;
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test action through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FBridgeTestAction ReadBridgeTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FBridgeTestAction
ReadBridgeTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<EBridgeTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          BridgeTestActionKinds()),
      func::map_array<TSharedPtr<FJsonObject>, FDirectiveRuleSet>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("rulesets")),
          ReadBridgeTestRuleset),
      DataAdapters::ReadStringArrayField(Value, TEXT("presetIds")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("targetId")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("valid")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("reason")),
  };
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test expected through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FBridgeTestExpected ReadBridgeTestExpected(const TSharedPtr<FJsonObject> &Object) */
inline FBridgeTestExpected
ReadBridgeTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("status")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasValidation")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("validationValid")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("validationReason")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("activeCount")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("activeFirstId")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("availableCount")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("availableFirstId")),
      DataAdapters::ReadOptionalStringField(
          Value, TEXT("availableSelectedRulesetId")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("presetIdCount")),
  };
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test step through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FBridgeTestStep ReadBridgeTestStep(const TSharedPtr<FJsonObject> &Object) */
inline FBridgeTestStep
ReadBridgeTestStep(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      ReadBridgeTestAction(
          DataAdapters::ReadObjectField(Value, TEXT("action"))),
      ReadBridgeTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test scenario through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FBridgeTestScenario ReadBridgeTestScenario(const TSharedPtr<FJsonObject> &Object) */
inline FBridgeTestScenario
ReadBridgeTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FBridgeTestStep>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("steps")),
          ReadBridgeTestStep),
  };
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test scenarios through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline TArray<FBridgeTestScenario> ReadBridgeTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FBridgeTestScenario>
ReadBridgeTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FBridgeTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadBridgeTestScenario);
}

/** User Story: As a features testing bridge consumer, I need to invoke read bridge test labels through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline FBridgeTestLabels ReadBridgeTestLabels(const DataAdapters::FSettingsSource &Source) */
inline FBridgeTestLabels
ReadBridgeTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("suite")),
      DataAdapters::ReadStringField(Labels, TEXT("caseName")),
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("status")),
      DataAdapters::ReadStringField(Labels, TEXT("error")),
      DataAdapters::ReadStringField(Labels, TEXT("hasValidation")),
      DataAdapters::ReadStringField(Labels, TEXT("validationValid")),
      DataAdapters::ReadStringField(Labels, TEXT("validationReason")),
      DataAdapters::ReadStringField(Labels, TEXT("activeCount")),
      DataAdapters::ReadStringField(Labels, TEXT("activeFirstId")),
      DataAdapters::ReadStringField(Labels, TEXT("availableCount")),
      DataAdapters::ReadStringField(Labels, TEXT("availableFirstId")),
      DataAdapters::ReadStringField(
          Labels, TEXT("availableSelectedRulesetId")),
      DataAdapters::ReadStringField(Labels, TEXT("presetIdCount")),
  };
}

/** User Story: As a features testing bridge consumer, I need to invoke testing bridge fixtures through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline const FBridgeTestFixtures &TestingBridgeFixtures() */
inline const FBridgeTestFixtures &TestingBridgeFixtures() {
  static const DataAdapters::FSettingsSource Settings =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/bridge/settings.json"));
  static const TArray<DataAdapters::FArraySource> Catalogs = {
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/bridge/scenarios/lifecycle.json")),
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/bridge/scenarios/catalogs.json")),
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/bridge/scenarios/resilience.json")),
  };
  static const FBridgeTestFixtures Fixtures = {
      ReadBridgeTestLabels(Settings),
      func::concat_arrays<FBridgeTestScenario>(
          func::map_array<DataAdapters::FArraySource,
                          TArray<FBridgeTestScenario>>(
              Catalogs, ReadBridgeTestScenarios)),
  };
  return Fixtures;
}

/** User Story: As a features testing bridge consumer, I need to invoke find bridge test scenario through a stable signature so the features testing bridge workflow remains explicit and composable. @fn inline func::Maybe<FBridgeTestScenario> FindBridgeTestScenario(const FString &Name) */
inline func::Maybe<FBridgeTestScenario>
FindBridgeTestScenario(const FString &Name) {
  return func::find_array<FBridgeTestScenario>(
      TestingBridgeFixtures().Scenarios,
      [&Name](const FBridgeTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Bridge
