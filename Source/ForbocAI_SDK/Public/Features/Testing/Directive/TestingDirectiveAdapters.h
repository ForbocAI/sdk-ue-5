#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Directive/TestingDirectiveTypes.h"

namespace Testing::Directive {

inline FDirectiveTestAction
ReadDirectiveTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(EDirectiveTestActionKind::Start));
  check(Kind < static_cast<int32>(EDirectiveTestActionKind::Count));
  return {
      static_cast<EDirectiveTestActionKind>(Kind),
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

inline TArray<FDirectiveTestScenario>
ReadDirectiveTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>,
                         FDirectiveTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadDirectiveTestScenario);
}

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

inline func::Maybe<FDirectiveTestScenario>
FindDirectiveTestScenario(const FString &Name) {
  return func::find_array<FDirectiveTestScenario>(
      DirectiveTestFixtures().Scenarios,
      [&Name](const FDirectiveTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::Directive
