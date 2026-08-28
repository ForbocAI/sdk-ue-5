#pragma once

#include "MicroGame/Features/Components/Harness/Maze/Definition/MazeDefinitionTypes.h"

namespace MicroGame::MazeGhost {

struct FMazeGhostConfig {
  FString Group;
  FString RunCommand;
  FString StopCommand;
  FString ResultsCommand;
  FString AnalysisHeader;
  FString AnalysisFooter;
  Maze::FMazeCommands Commands;
  FString CommandFailed;
  FString LogFileEnvKey;
  FString DefaultLogPath;
  FString OutputToken;
};

} // namespace MicroGame::MazeGhost
