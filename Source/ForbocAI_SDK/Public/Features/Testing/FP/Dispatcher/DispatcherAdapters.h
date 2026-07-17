#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/FP/Dispatcher/DispatcherTypes.h"

namespace Testing::FP::Dispatcher {

/** User Story: As a testing fp dispatcher consumer, I need to invoke read entry through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FEntryFixture ReadEntry(const TSharedRef<FJsonObject> &Object) */
inline FEntryFixture ReadEntry(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("key")),
      DataAdapters::ReadNumberField(Object, TEXT("value")),
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read asserted entry through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FAssertedEntryFixture ReadAssertedEntry(const TSharedPtr<FJsonObject> &Object) */
inline FAssertedEntryFixture
ReadAssertedEntry(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  return {
      DataAdapters::ReadStringField(Object.ToSharedRef(), TEXT("key")),
      DataAdapters::ReadNumberField(Object.ToSharedRef(), TEXT("value")),
      DataAdapters::ReadStringField(Object.ToSharedRef(),
                                    TEXT("presentLabel")),
      DataAdapters::ReadStringField(Object.ToSharedRef(), TEXT("valueLabel")),
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read has entry through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FHasEntryFixture ReadHasEntry(const TSharedPtr<FJsonObject> &Object) */
inline FHasEntryFixture
ReadHasEntry(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  return {
      DataAdapters::ReadStringField(Object.ToSharedRef(), TEXT("key")),
      DataAdapters::ReadNumberField(Object.ToSharedRef(), TEXT("value")),
      DataAdapters::ReadStringField(Object.ToSharedRef(),
                                    TEXT("presentLabel")),
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read key lookup fixture through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FKeyLookupFixture ReadKeyLookupFixture(const DataAdapters::FSettingsSource &Source) */
inline FKeyLookupFixture
ReadKeyLookupFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("keyLookup"));
  return {func::map_array<TSharedPtr<FJsonObject>, FAssertedEntryFixture>(
      DataAdapters::ReadObjectArrayField(Object, TEXT("entries")),
      ReadAssertedEntry)};
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read missing key fixture through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FMissingKeyFixture ReadMissingKeyFixture(const DataAdapters::FSettingsSource &Source) */
inline FMissingKeyFixture
ReadMissingKeyFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("missingKey"));
  return {
      ReadEntry(DataAdapters::ReadObjectField(Object, TEXT("entry"))),
      DataAdapters::ReadStringField(Object, TEXT("missingKey")),
      DataAdapters::ReadStringField(Object, TEXT("label")),
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read has and keys fixture through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FHasAndKeysFixture ReadHasAndKeysFixture(const DataAdapters::FSettingsSource &Source) */
inline FHasAndKeysFixture
ReadHasAndKeysFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("hasAndKeys"));
  return {
      func::map_array<TSharedPtr<FJsonObject>, FHasEntryFixture>(
          DataAdapters::ReadObjectArrayField(Object, TEXT("entries")),
          ReadHasEntry),
      DataAdapters::ReadStringField(Object, TEXT("missingKey")),
      DataAdapters::ReadStringField(Object, TEXT("missingLabel")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedCount")),
      DataAdapters::ReadStringField(Object, TEXT("countLabel")),
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read either miss fixture through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FEitherMissFixture ReadEitherMissFixture(const DataAdapters::FSettingsSource &Source) */
inline FEitherMissFixture
ReadEitherMissFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("eitherMiss"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      ReadEntry(DataAdapters::ReadObjectField(Object, TEXT("entry"))),
      DataAdapters::ReadStringField(Object, TEXT("missingKey")),
      DataAdapters::ReadStringField(Object, TEXT("error")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("hitSide")),
          DataAdapters::ReadStringField(Labels, TEXT("hitValue")),
          DataAdapters::ReadStringField(Labels, TEXT("missSide")),
          DataAdapters::ReadStringField(Labels, TEXT("missError")),
      },
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke read arg strict fixture through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline FArgStrictFixture ReadArgStrictFixture(const DataAdapters::FSettingsSource &Source) */
inline FArgStrictFixture
ReadArgStrictFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("argStrict"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("registeredKey")),
      DataAdapters::ReadStringField(Object, TEXT("missingKey")),
      DataAdapters::ReadNumberField(Object, TEXT("input")),
      DataAdapters::ReadNumberField(Object, TEXT("matchedInput")),
      DataAdapters::ReadStringField(Object, TEXT("matchedOutput")),
      DataAdapters::ReadStringField(Object, TEXT("otherOutput")),
      DataAdapters::ReadStringField(Object, TEXT("missingError")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("hitPresent")),
          DataAdapters::ReadStringField(Labels, TEXT("hitValue")),
          DataAdapters::ReadStringField(Labels, TEXT("missSide")),
          DataAdapters::ReadStringField(Labels, TEXT("missError")),
      },
  };
}

/** User Story: As a testing fp dispatcher consumer, I need to invoke dispatcher fixtures through a stable signature so the testing fp dispatcher workflow remains explicit and composable. @fn inline const FDispatcherFixtures &DispatcherFixtures() */
inline const FDispatcherFixtures &DispatcherFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/fp/dispatcher.json"));
  static const FDispatcherFixtures Fixtures = {
      ReadKeyLookupFixture(Source),
      ReadMissingKeyFixture(Source),
      ReadHasAndKeysFixture(Source),
      ReadEitherMissFixture(Source),
      ReadArgStrictFixture(Source),
  };
  return Fixtures;
}

} // namespace Testing::FP::Dispatcher
