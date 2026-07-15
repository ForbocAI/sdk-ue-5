#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Bridge/TestingBridgeTypes.h"

namespace Testing::Bridge {

inline FDirectiveRuleSet
ReadBridgeTestRuleset(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FDirectiveRuleSet Ruleset;
  Ruleset.Id = DataAdapters::ReadStringField(Value, TEXT("id"));
  Ruleset.RulesetId =
      DataAdapters::ReadStringField(Value, TEXT("rulesetId"));
  return Ruleset;
}

inline FBridgeTestAction
ReadBridgeTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(
                    EBridgeTestActionKind::ValidationRequested));
  check(Kind < static_cast<int32>(EBridgeTestActionKind::Count));
  return {
      static_cast<EBridgeTestActionKind>(Kind),
      func::map_array<TSharedPtr<FJsonObject>, FDirectiveRuleSet>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("rulesets")),
          ReadBridgeTestRuleset),
      DataAdapters::ReadStringArrayField(Value, TEXT("presetIds")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("targetId")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("valid")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("reason")),
  };
}

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

inline TArray<FBridgeTestScenario>
ReadBridgeTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FBridgeTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadBridgeTestScenario);
}

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

inline func::Maybe<FBridgeTestScenario>
FindBridgeTestScenario(const FString &Name) {
  return func::find_array<FBridgeTestScenario>(
      TestingBridgeFixtures().Scenarios,
      [&Name](const FBridgeTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Bridge
