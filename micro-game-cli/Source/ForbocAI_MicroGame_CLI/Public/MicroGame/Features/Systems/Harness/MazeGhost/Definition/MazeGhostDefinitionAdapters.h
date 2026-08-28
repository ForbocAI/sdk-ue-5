#pragma once

#include "MicroGame/Features/Components/Harness/MazeGhost/Definition/MazeGhostDefinitionTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::MazeGhost {

/** User Story: As Maze Ghost orchestration, I need one immutable authored definition whose operation shape is the normal Maze shape with Ghost-only command values. @fn inline const FMazeGhostConfig &MazeGhostConfig() */
inline const FMazeGhostConfig &MazeGhostConfig() {
  static const FMazeGhostConfig Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("harness/maze-ghost.json"));
    const TSharedRef<FJsonObject> Root = Source.Root;
    return FMazeGhostConfig{
        DataAdapters::ReadStringField(Root, TEXT("group")),
        DataAdapters::ReadStringField(Root, TEXT("runCommand")),
        DataAdapters::ReadStringField(Root, TEXT("stopCommand")),
        DataAdapters::ReadStringField(Root, TEXT("resultsCommand")),
        DataAdapters::ReadStringField(Root, TEXT("analysisHeader")),
        DataAdapters::ReadStringField(Root, TEXT("analysisFooter")),
        Maze::ReadMazeCommands(
            DataAdapters::ReadObjectField(Source, TEXT("commands"))),
        DataAdapters::ReadStringField(Root, TEXT("commandFailed")),
        DataAdapters::ReadStringField(Root, TEXT("logFileEnvKey")),
        DataAdapters::ReadStringField(Root, TEXT("defaultLogPath")),
        DataAdapters::ReadStringField(
            DataAdapters::ReadObjectField(Source, TEXT("tokens")),
            TEXT("output")),
    };
  }();
  return Data;
}

} // namespace MicroGame::MazeGhost
