#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/FP/Match/MatchTypes.h"

namespace Testing::FP::Match {

inline FResultLabels
ReadResultLabels(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("present")),
      DataAdapters::ReadStringField(Object, TEXT("value")),
  };
}

inline FPredicateCaseFixture
ReadPredicateCase(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadNumberField(Object, TEXT("input")),
      DataAdapters::ReadStringField(Object, TEXT("expected")),
      ReadResultLabels(DataAdapters::ReadObjectField(Object, TEXT("labels"))),
  };
}

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

inline FEqualsCaseFixture
ReadEqualsCase(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("input")),
      DataAdapters::ReadNumberField(Object, TEXT("result")),
      ReadResultLabels(DataAdapters::ReadObjectField(Object, TEXT("labels"))),
  };
}

inline FEqualsRegisteredFixture
ReadEqualsRegistered(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("input")),
      DataAdapters::ReadNumberField(Object, TEXT("result")),
  };
}

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
