#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Maze {

struct FMaze {
  TMap<FString, TMap<FString, FString>> Cells;
  TArray<FString> CellOrder;
  FString Start;
  FString Exit;
  TArray<FString> DeadEnds;
  TMap<FString, int32> Distances;
  int32 Optimal{};
  uint32 Seed{};
};

struct FRandomStep {
  uint32 State{};
  double Value{};
};

struct FIntegerPick {
  uint32 State{};
  int32 Value{};
};

struct FShuffleResult {
  uint32 State{};
  TArray<FString> Values;
};

struct FPassageResult {
  uint32 State{};
  TMap<FString, TMap<FString, FString>> Cells;
  bool bOpened{};
};

struct FDeadEndResult {
  uint32 State{};
  TMap<FString, TMap<FString, FString>> Cells;
  TArray<FString> CellOrder;
  FString Terminal;
};

struct FMazeGenerationState {
  uint32 RandomState{};
  TMap<FString, TMap<FString, FString>> Cells;
  TArray<FString> CellOrder;
  TArray<FString> DeadEnds;
};

struct FBreadthFirstState {
  TArray<FString> Frontier;
  TMap<FString, int32> Distances;
};

struct FWardOutcome {
  TArray<FString> Options;
  TArray<FString> VisitedDirections;
  TArray<FString> AvoidDirections;
  TArray<FString> WardDirections;
  bool bRealChoice{};
  TArray<FString> OptimalDirections;
};

struct FDirectionHit {
  FString Direction;
  int32 Index{};
};

} // namespace MicroGame::Maze
