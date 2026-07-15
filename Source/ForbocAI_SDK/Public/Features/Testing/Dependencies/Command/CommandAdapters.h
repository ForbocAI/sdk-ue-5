#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/Dependencies/Command/CommandTypes.h"

namespace Testing::Dependencies::Command {

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
