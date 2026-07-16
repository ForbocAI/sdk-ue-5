#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Dependencies/Command/CommandTypes.h"

namespace Testing::Dependencies::Command {

/** User Story: As a testing dependencies command consumer, I need to invoke read setup test command through a stable signature so the testing dependencies command workflow remains explicit and composable. @fn inline FSetupTestCommand ReadSetupTestCommand(const TSharedPtr<FJsonObject> &Object) */
inline FSetupTestCommand
ReadSetupTestCommand(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("key")),
      DataAdapters::ReadStringArrayField(Value, TEXT("arguments")),
      DataAdapters::ReadStringField(Value, TEXT("label")),
  };
}

/** User Story: As a testing dependencies command consumer, I need to invoke setup test fixtures through a stable signature so the testing dependencies command workflow remains explicit and composable. @fn inline const FSetupTestFixtures &SetupTestFixtures() */
inline const FSetupTestFixtures &SetupTestFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/dependencies/setup.json"));
  static const FSetupTestFixtures Fixtures = {
      func::map_array<TSharedPtr<FJsonObject>, FSetupTestCommand>(
          DataAdapters::ReadObjectArrayField(Source.Root, TEXT("commands")),
          ReadSetupTestCommand),
  };
  return Fixtures;
}

} // namespace Testing::Dependencies::Command
