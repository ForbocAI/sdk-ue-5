#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/NPC/TestingNPCTypes.h"

namespace Testing::NPC {

inline FNPCHistoryEntry
ReadNPCTestHistoryEntry(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FNPCHistoryEntry Entry;
  Entry.Role = DataAdapters::ReadStringField(Value, TEXT("role"));
  Entry.Content = DataAdapters::ReadStringField(Value, TEXT("content"));
  return Entry;
}

inline FNPCTestAction
ReadNPCTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  const int32 Kind = DataAdapters::ReadNumberField(Value, TEXT("kind"));
  check(Kind >= static_cast<int32>(ENPCTestActionKind::InfoReceived));
  check(Kind < static_cast<int32>(ENPCTestActionKind::Count));
  return {
      static_cast<ENPCTestActionKind>(Kind),
      DataAdapters::ReadOptionalStringField(Value, TEXT("id")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("persona")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("stateJson")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("deltaJson")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("role")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("content")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("reason")),
      func::map_array<TSharedPtr<FJsonObject>, FNPCHistoryEntry>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("history")),
          ReadNPCTestHistoryEntry),
  };
}

inline FNPCTestExpected
ReadNPCTestExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadOptionalStringField(Value, TEXT("activeId")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("npcCount")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("hasNpc")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("selectedPersona")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("selectedStateJson")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("historyCount")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("blocked")),
      DataAdapters::ReadOptionalStringField(Value, TEXT("blockReason")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("stateLogCount")),
      DataAdapters::ReadOptionalBooleanField(Value, TEXT("activeExists")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("idCount")),
      DataAdapters::ReadOptionalNumberField(Value, TEXT("entityCount")),
  };
}

inline FNPCTestStep ReadNPCTestStep(
    const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      ReadNPCTestAction(DataAdapters::ReadObjectField(Value, TEXT("action"))),
      ReadNPCTestExpected(
          DataAdapters::ReadObjectField(Value, TEXT("expected"))),
  };
}

inline FNPCTestScenario
ReadNPCTestScenario(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      DataAdapters::ReadStringField(Value, TEXT("name")),
      func::map_array<TSharedPtr<FJsonObject>, FNPCTestStep>(
          DataAdapters::ReadObjectArrayField(Value, TEXT("steps")),
          ReadNPCTestStep),
  };
}

inline TArray<FNPCTestScenario>
ReadNPCTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FNPCTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadNPCTestScenario);
}

inline FNPCTestLabels
ReadNPCTestLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Labels, TEXT("suite")),
      DataAdapters::ReadStringField(Labels, TEXT("caseName")),
      DataAdapters::ReadStringField(Labels, TEXT("requiredField")),
      DataAdapters::ReadStringField(Labels, TEXT("scenarioPresent")),
      DataAdapters::ReadStringField(Labels, TEXT("activeId")),
      DataAdapters::ReadStringField(Labels, TEXT("npcCount")),
      DataAdapters::ReadStringField(Labels, TEXT("hasNpc")),
      DataAdapters::ReadStringField(Labels, TEXT("selectedPersona")),
      DataAdapters::ReadStringField(Labels, TEXT("selectedStateJson")),
      DataAdapters::ReadStringField(Labels, TEXT("historyCount")),
      DataAdapters::ReadStringField(Labels, TEXT("blocked")),
      DataAdapters::ReadStringField(Labels, TEXT("blockReason")),
      DataAdapters::ReadStringField(Labels, TEXT("stateLogCount")),
      DataAdapters::ReadStringField(Labels, TEXT("activeExists")),
      DataAdapters::ReadStringField(Labels, TEXT("idCount")),
      DataAdapters::ReadStringField(Labels, TEXT("entityCount")),
  };
}

inline const FNPCTestFixtures &TestingNPCFixtures() {
  static const DataAdapters::FSettingsSource Settings =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/npc/settings.json"));
  static const TArray<DataAdapters::FArraySource> Catalogs = {
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/npc/scenarios/lifecycle.json")),
      DataAdapters::ArraySource(
          TEXT("ForbocAI_SDK"),
          TEXT("Data/tests/npc/scenarios/state.json")),
  };
  static const FNPCTestFixtures Fixtures = {
      ReadNPCTestLabels(Settings),
      func::concat_arrays<FNPCTestScenario>(
          func::map_array<DataAdapters::FArraySource,
                          TArray<FNPCTestScenario>>(
              Catalogs, ReadNPCTestScenarios)),
  };
  return Fixtures;
}

inline func::Maybe<FNPCTestScenario>
FindNPCTestScenario(const FString &Name) {
  return func::find_array<FNPCTestScenario>(
      TestingNPCFixtures().Scenarios,
      [&Name](const FNPCTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::NPC
