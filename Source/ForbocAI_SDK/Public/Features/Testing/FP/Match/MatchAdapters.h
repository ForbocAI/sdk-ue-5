#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/FP/Match/MatchTypes.h"

namespace Testing::FP::Match {

/** User Story: As a testing fp match consumer, I need to invoke read result labels through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FResultLabels ReadResultLabels(const TSharedRef<FJsonObject> &Object) */
inline FResultLabels
ReadResultLabels(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("present")),
      DataAdapters::ReadStringField(Object, TEXT("value")),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read predicate case through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FPredicateCaseFixture ReadPredicateCase(const TSharedRef<FJsonObject> &Object) */
inline FPredicateCaseFixture
ReadPredicateCase(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadNumberField(Object, TEXT("input")),
      DataAdapters::ReadStringField(Object, TEXT("expected")),
      ReadResultLabels(DataAdapters::ReadObjectField(Object, TEXT("labels"))),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read predicate fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FPredicateFixture ReadPredicateFixture(const DataAdapters::FSettingsSource &Source) */
inline FPredicateFixture
ReadPredicateFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("predicate"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("pivot")),
      ReadPredicateCase(
          DataAdapters::ReadObjectField(Object, TEXT("negative"))),
      ReadPredicateCase(DataAdapters::ReadObjectField(Object, TEXT("zero"))),
      ReadPredicateCase(
          DataAdapters::ReadObjectField(Object, TEXT("positive"))),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read wildcard fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FWildcardFixture ReadWildcardFixture(const DataAdapters::FSettingsSource &Source) */
inline FWildcardFixture
ReadWildcardFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("wildcard"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("exactInput")),
      DataAdapters::ReadStringField(Object, TEXT("exactExpected")),
      DataAdapters::ReadNumberField(Object, TEXT("fallbackInput")),
      DataAdapters::ReadStringField(Object, TEXT("fallbackExpected")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("exactPresent")),
          DataAdapters::ReadStringField(Labels, TEXT("exactValue")),
          DataAdapters::ReadStringField(Labels, TEXT("fallbackPresent")),
          DataAdapters::ReadStringField(Labels, TEXT("fallbackValue")),
      },
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read no match fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FNoMatchFixture ReadNoMatchFixture(const DataAdapters::FSettingsSource &Source) */
inline FNoMatchFixture
ReadNoMatchFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("noMatch"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("registeredInput")),
      DataAdapters::ReadStringField(Object, TEXT("registeredResult")),
      DataAdapters::ReadNumberField(Object, TEXT("missingInput")),
      DataAdapters::ReadStringField(Object, TEXT("label")),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read equals case through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FEqualsCaseFixture ReadEqualsCase(const TSharedRef<FJsonObject> &Object) */
inline FEqualsCaseFixture
ReadEqualsCase(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("input")),
      DataAdapters::ReadNumberField(Object, TEXT("result")),
      ReadResultLabels(DataAdapters::ReadObjectField(Object, TEXT("labels"))),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read equals registered through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FEqualsRegisteredFixture ReadEqualsRegistered(const TSharedRef<FJsonObject> &Object) */
inline FEqualsRegisteredFixture
ReadEqualsRegistered(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("input")),
      DataAdapters::ReadNumberField(Object, TEXT("result")),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read equals fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FEqualsFixture ReadEqualsFixture(const DataAdapters::FSettingsSource &Source) */
inline FEqualsFixture
ReadEqualsFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("equals"));
  return {
      ReadEqualsCase(DataAdapters::ReadObjectField(Object, TEXT("first"))),
      ReadEqualsCase(DataAdapters::ReadObjectField(Object, TEXT("second"))),
      ReadEqualsRegistered(
          DataAdapters::ReadObjectField(Object, TEXT("third"))),
      DataAdapters::ReadStringField(Object, TEXT("missingInput")),
      DataAdapters::ReadStringField(Object, TEXT("missingLabel")),
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read nullable pointer fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FNullablePointerFixture ReadNullablePointerFixture(const DataAdapters::FSettingsSource &Source) */
inline FNullablePointerFixture
ReadNullablePointerFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("nullablePointer"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("value")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("present")),
          DataAdapters::ReadStringField(Labels, TEXT("value")),
          DataAdapters::ReadStringField(Labels, TEXT("missing")),
      },
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read nullable value fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FNullableValueFixture ReadNullableValueFixture(const DataAdapters::FSettingsSource &Source) */
inline FNullableValueFixture
ReadNullableValueFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("nullableValue"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("validValue")),
      DataAdapters::ReadStringField(Object, TEXT("emptyValue")),
      DataAdapters::ReadBooleanField(Object, TEXT("validFlag")),
      DataAdapters::ReadBooleanField(Object, TEXT("invalidFlag")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("present")),
          DataAdapters::ReadStringField(Labels, TEXT("value")),
          DataAdapters::ReadStringField(Labels, TEXT("missing")),
      },
  };
}

/** User Story: As a testing fp match consumer, I need to invoke read require just fixture through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline FRequireJustFixture ReadRequireJustFixture(const DataAdapters::FSettingsSource &Source) */
inline FRequireJustFixture
ReadRequireJustFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("requireJust"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("value")),
      DataAdapters::ReadStringField(Object, TEXT("successMessage")),
      DataAdapters::ReadStringField(Object, TEXT("failureMessage")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("value")),
          DataAdapters::ReadStringField(Labels, TEXT("error")),
          DataAdapters::ReadStringField(Labels, TEXT("throws")),
      },
  };
}

/** User Story: As a testing fp match consumer, I need to invoke match fixtures through a stable signature so the testing fp match workflow remains explicit and composable. @fn inline const FMatchFixtures &MatchFixtures() */
inline const FMatchFixtures &MatchFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/fp/match.json"));
  static const FMatchFixtures Fixtures = {
      ReadPredicateFixture(Source),
      ReadWildcardFixture(Source),
      ReadNoMatchFixture(Source),
      ReadEqualsFixture(Source),
      ReadNullablePointerFixture(Source),
      ReadNullableValueFixture(Source),
      ReadRequireJustFixture(Source),
  };
  return Fixtures;
}

} // namespace Testing::FP::Match
