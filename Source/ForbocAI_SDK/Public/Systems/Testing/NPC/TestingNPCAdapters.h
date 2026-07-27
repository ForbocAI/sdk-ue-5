#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Systems/Testing/Fixture/FixtureAdapters.h"
#include "Components/Testing/NPC/TestingNPCTypes.h"

namespace Testing::NPC {

/**
 * User Story: As an NPC reducer-test consumer, I need the complete semantic action registry so fixture behavior cannot drift when enum declarations change.
 * @fn inline const TArray<Testing::Fixture::TTestingActionKind<ENPCTestActionKind>> & NPCTestActionKinds()
 */
inline const TArray<Testing::Fixture::TTestingActionKind<ENPCTestActionKind>> &
NPCTestActionKinds() {
#define FORBOC_NPC_TEST_ACTION_KIND(Name) {FString(TEXT(#Name)), ENPCTestActionKind::Name}
  static const TArray<Testing::Fixture::TTestingActionKind<ENPCTestActionKind>>
      Kinds = {
          FORBOC_NPC_TEST_ACTION_KIND(InfoReceived),
          FORBOC_NPC_TEST_ACTION_KIND(ActiveChanged),
          FORBOC_NPC_TEST_ACTION_KIND(StateReplaced),
          FORBOC_NPC_TEST_ACTION_KIND(StateUpdated),
          FORBOC_NPC_TEST_ACTION_KIND(HistoryAppended),
          FORBOC_NPC_TEST_ACTION_KIND(HistoryReceived),
          FORBOC_NPC_TEST_ACTION_KIND(Blocked),
          FORBOC_NPC_TEST_ACTION_KIND(BlockCleared),
          FORBOC_NPC_TEST_ACTION_KIND(Removed),
          FORBOC_NPC_TEST_ACTION_KIND(Inspect),
      };
#undef FORBOC_NPC_TEST_ACTION_KIND
  check(Kinds.Num() == static_cast<int32>(ENPCTestActionKind::Count));
  return Kinds;
}

/** User Story: As a features testing npc consumer, I need to invoke read npctest history entry through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCHistoryEntry ReadNPCTestHistoryEntry(const TSharedPtr<FJsonObject> &Object) */
inline FNPCHistoryEntry
ReadNPCTestHistoryEntry(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FNPCHistoryEntry Entry;
  Entry.Role = DataAdapters::ReadStringField(Value, TEXT("role"));
  Entry.Content = DataAdapters::ReadStringField(Value, TEXT("content"));
  return Entry;
}

/** User Story: As a features testing npc consumer, I need to invoke read npctest action through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCTestAction ReadNPCTestAction(const TSharedPtr<FJsonObject> &Object) */
inline FNPCTestAction
ReadNPCTestAction(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  return {
      Testing::Fixture::ReadTestingActionKind<ENPCTestActionKind>(
          DataAdapters::ReadStringField(Value, TEXT("kind")),
          NPCTestActionKinds()),
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

/** User Story: As a features testing npc consumer, I need to invoke read npctest expected through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCTestExpected ReadNPCTestExpected(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing npc consumer, I need to invoke read npctest step through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCTestStep ReadNPCTestStep( const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing npc consumer, I need to invoke read npctest scenario through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCTestScenario ReadNPCTestScenario(const TSharedPtr<FJsonObject> &Object) */
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

/** User Story: As a features testing npc consumer, I need to invoke read npctest scenarios through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline TArray<FNPCTestScenario> ReadNPCTestScenarios(const DataAdapters::FArraySource &Source) */
inline TArray<FNPCTestScenario>
ReadNPCTestScenarios(const DataAdapters::FArraySource &Source) {
  return func::map_array<TSharedPtr<FJsonObject>, FNPCTestScenario>(
      DataAdapters::ReadObjectArray(Source), ReadNPCTestScenario);
}

/** User Story: As a features testing npc consumer, I need to invoke read npctest labels through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline FNPCTestLabels ReadNPCTestLabels(const DataAdapters::FSettingsSource &Source) */
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

/** User Story: As a features testing npc consumer, I need to invoke testing npcfixtures through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline const FNPCTestFixtures &TestingNPCFixtures() */
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

/** User Story: As a features testing npc consumer, I need to invoke find npctest scenario through a stable signature so the features testing npc workflow remains explicit and composable. @fn inline func::Maybe<FNPCTestScenario> FindNPCTestScenario(const FString &Name) */
inline func::Maybe<FNPCTestScenario>
FindNPCTestScenario(const FString &Name) {
  return func::find_array<FNPCTestScenario>(
      TestingNPCFixtures().Scenarios,
      [&Name](const FNPCTestScenario &Scenario) {
        return Scenario.Name == Name;
      });
}

} // namespace Testing::NPC
