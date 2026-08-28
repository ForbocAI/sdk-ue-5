#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Testing::MazeGhost {

struct FMazeGhostTestingLabels {
  FString OperationShape;
  FString GhostOnly;
  FString Deterministic;
  FString Topology;
  FString Reciprocal;
  FString Distance;
  FString Ward;
  FString ModeVocabulary;
};

struct FMazeGhostTestingData {
  FString AutomationName;
  FMazeGhostTestingLabels Labels;
  uint32 Seed{};
  FString CommandPrefix;
  FString ForbiddenFragment;
  int32 ExpectedOperationCount{};
  int32 ExpectedLifecycleCommandCount{};
  FString UsageSeparator;
};

} // namespace MicroGame::Testing::MazeGhost
