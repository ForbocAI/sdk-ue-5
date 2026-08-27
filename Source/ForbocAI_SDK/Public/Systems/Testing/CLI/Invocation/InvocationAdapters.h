#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/CLI/Invocation/InvocationTypes.h"

namespace Testing::CLI::Invocation {

/** User Story: As a testing cli invocation consumer, I need to invoke read invocation test labels through a stable signature so the testing cli invocation workflow remains explicit and composable. @fn inline FInvocationTestLabels ReadInvocationTestLabels( const TSharedRef<FJsonObject> &Labels) */
inline FInvocationTestLabels ReadInvocationTestLabels(
    const TSharedRef<FJsonObject> &Labels) {
  return {
      DataAdapters::ReadStringField(Labels, TEXT("nodeCommandCount")),
      DataAdapters::ReadStringField(Labels, TEXT("commandMatched")),
      DataAdapters::ReadStringField(Labels, TEXT("commandKey")),
      DataAdapters::ReadStringField(Labels, TEXT("arguments")),
      DataAdapters::ReadStringField(Labels, TEXT("apiUrl")),
      DataAdapters::ReadStringField(Labels, TEXT("apiKey")),
  };
}

/** User Story: As a testing cli invocation consumer, I need to invoke read invocation test scenario through a stable signature so the testing cli invocation workflow remains explicit and composable. @fn inline FInvocationTestScenario ReadInvocationTestScenario( const TSharedPtr<FJsonObject> &Object) */
inline FInvocationTestScenario ReadInvocationTestScenario(
    const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      DataAdapters::ReadStringField(Value, TEXT("commandletParams")),
      DataAdapters::ReadBooleanField(Value, TEXT("expectedMatched")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("expectedKey")),
      DataAdapters::ReadStringArrayField(Value, TEXT("expectedArgs")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("expectedApiUrl")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("expectedApiKey")),
  };
}

/** User Story: As a testing cli invocation consumer, I need to invoke invocation test fixtures through a stable signature so the testing cli invocation workflow remains explicit and composable. @fn inline const FInvocationTestFixtures &InvocationTestFixtures() */
inline const FInvocationTestFixtures &InvocationTestFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/cli/invocation.json"));
  static const FInvocationTestFixtures Fixtures = {
      ReadInvocationTestLabels(
          DataAdapters::ReadObjectField(Source, TEXT("labels"))),
      func::map_array<TSharedPtr<FJsonObject>, FInvocationTestScenario>(
          DataAdapters::ReadObjectArrayField(Source.Root,
                                             TEXT("invocations")),
          ReadInvocationTestScenario),
  };
  return Fixtures;
}

} // namespace Testing::CLI::Invocation
