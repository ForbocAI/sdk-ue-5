#pragma once

#include "MicroGame/Features/Components/Testing/Harness/MazeGhost/Fixture/MazeGhostFixtureTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::Testing::MazeGhost {

/** User Story: As Maze Ghost benchmark tests, I need registration, labels, seed, command policy, and counts loaded from one authored fixture. @fn inline const FMazeGhostTestingData &MazeGhostTestingData() */
inline const FMazeGhostTestingData &MazeGhostTestingData() {
  static const FMazeGhostTestingData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("tests/harness/maze-ghost.json"));
    const TSharedRef<FJsonObject> Labels =
        DataAdapters::ReadObjectField(Source, TEXT("labels"));
    FMazeGhostTestingData Value;
    Value.AutomationName =
        DataAdapters::ReadStringField(Source.Root, TEXT("automationName"));
    Value.Labels = {
        DataAdapters::ReadStringField(Labels, TEXT("operationShape")),
        DataAdapters::ReadStringField(Labels, TEXT("ghostOnly")),
        DataAdapters::ReadStringField(Labels, TEXT("deterministic")),
        DataAdapters::ReadStringField(Labels, TEXT("topology")),
        DataAdapters::ReadStringField(Labels, TEXT("reciprocal")),
        DataAdapters::ReadStringField(Labels, TEXT("distance")),
        DataAdapters::ReadStringField(Labels, TEXT("ward")),
        DataAdapters::ReadStringField(Labels, TEXT("modeVocabulary")),
    };
    Value.Seed = static_cast<uint32>(
        DataAdapters::ReadNumberField(Source.Root, TEXT("seed")));
    Value.CommandPrefix =
        DataAdapters::ReadStringField(Source.Root, TEXT("commandPrefix"));
    Value.ForbiddenFragment =
        DataAdapters::ReadStringField(Source.Root, TEXT("forbiddenFragment"));
    Value.ExpectedOperationCount = DataAdapters::ReadNumberField(
        Source.Root, TEXT("expectedOperationCount"));
    Value.ExpectedLifecycleCommandCount = DataAdapters::ReadNumberField(
        Source.Root, TEXT("expectedLifecycleCommandCount"));
    Value.UsageSeparator =
        DataAdapters::ReadStringField(Source.Root, TEXT("usageSeparator"));
    return Value;
  }();
  return Data;
}

} // namespace MicroGame::Testing::MazeGhost
