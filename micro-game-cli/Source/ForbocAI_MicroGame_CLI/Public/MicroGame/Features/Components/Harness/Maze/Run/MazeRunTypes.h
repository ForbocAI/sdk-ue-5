#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "CoreMinimal.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace MicroGame::Maze {

struct FMazeNpc {
  FString Id;
  FString DisplayName;
};

struct FMazeStep {
  bool bSuccess{true};
  FString Failure;
  FString Cell;
  int32 DeadEnds{};
  bool bSolved{};
  int32 Turns{};
  int32 RealChoices{};
  int32 OptimalChoices{};
  TArray<FString> Lines;
};

struct FMazeCommandResult {
  bool bSuccess{};
  CommandRunner::FCommandOutput Output;
  FString Failure;
};

struct FMazeNpcResult {
  bool bSuccess{};
  FMazeNpc Npc;
  FString Failure;
};

struct FMazeAttributesResult {
  bool bSuccess{};
  TArray<FString> Values;
  FString Failure;
};

struct FMazeRunResult {
  bool bSuccess{};
  TArray<FString> Lines;
  FString Failure;
};

struct FMazeLogTarget {
  FString EnvironmentKey;
  FString DefaultPath;
};

struct FMazeRunOptions {
  bool bDebug{};
  func::Maybe<uint32> Seed;
  FMazeLogTarget LogTarget;
  FString CommandFailureFormat;
};

} // namespace MicroGame::Maze
