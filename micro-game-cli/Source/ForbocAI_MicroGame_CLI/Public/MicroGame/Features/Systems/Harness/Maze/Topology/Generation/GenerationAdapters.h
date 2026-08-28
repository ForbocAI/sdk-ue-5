#pragma once

#include "HAL/PlatformTime.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Distance/DistanceAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Passage/PassageAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze generation, I need main-path identifiers assembled recursively from authored bounds. @fn inline TArray<FString> BuildMainPath(int32 Length, int32 Index, TArray<FString> Cells) */
inline TArray<FString> BuildMainPath(int32 Length, int32 Index,
                                    TArray<FString> Cells) {
  return Index >= Length
             ? Cells
             : (Cells.Add(Index == MazeConfig().Numbers.Zero
                              ? MazeConfig().StartId
                          : Index == Length - MazeConfig().Numbers.One
                              ? MazeConfig().ExitId
                              : FormatCellId(Index)),
                BuildMainPath(Length, Index + MazeConfig().Numbers.One,
                              MoveTemp(Cells)));
}

/**
 * User Story: As distinct trap placement, I need only interior main-path cells selected recursively as branch anchors.
 * @fn inline TArray<FString> SelectInteriorPath( const TArray<FString> &Path, int32 Index, TArray<FString> Cells)
 */
inline TArray<FString> SelectInteriorPath(
    const TArray<FString> &Path, int32 Index, TArray<FString> Cells) {
  return Index >= Path.Num() - MazeConfig().Numbers.One
             ? Cells
             : (Cells.Add(Path[Index]),
                SelectInteriorPath(Path,
                                   Index + MazeConfig().Numbers.One,
                                   MoveTemp(Cells)));
}

/**
 * User Story: As Maze generation, I need empty cells initialized from one ordered identifier list.
 * @fn inline FMazeGenerationState InitializeCells( const TArray<FString> &Path, int32 Index, FMazeGenerationState State)
 */
inline FMazeGenerationState InitializeCells(
    const TArray<FString> &Path, int32 Index, FMazeGenerationState State) {
  return Index >= Path.Num()
             ? State
             : (State.Cells.Add(Path[Index], {}),
                State.CellOrder.Add(Path[Index]),
                InitializeCells(Path, Index + MazeConfig().Numbers.One,
                                MoveTemp(State)));
}

/**
 * User Story: As Maze generation, I need the main route connected through reciprocal passage transitions.
 * @fn inline FMazeGenerationState ConnectMainPath( const TArray<FString> &Path, int32 Index, FMazeGenerationState State)
 */
inline FMazeGenerationState ConnectMainPath(
    const TArray<FString> &Path, int32 Index, FMazeGenerationState State) {
  return Index + MazeConfig().Numbers.One >= Path.Num()
             ? State
             : [&]() {
                 const FPassageResult Passage = OpenPassage(
                     State.Cells, State.RandomState, Path[Index],
                     Path[Index + MazeConfig().Numbers.One]);
                 State.RandomState = Passage.State;
                 State.Cells = Passage.Cells;
                 return ConnectMainPath(
                     Path, Index + MazeConfig().Numbers.One, MoveTemp(State));
               }();
}

/**
 * User Story: As Maze trap generation, I need one corridor extended recursively while retaining only cells connected to the anchor.
 * @fn inline FDeadEndResult ExtendDeadEnd( const FMazeGenerationState &State, int32 Branch, const FString &Previous, int32 Depth, const FString &Terminal)
 */
inline FDeadEndResult ExtendDeadEnd(
    const FMazeGenerationState &State, int32 Branch,
    const FString &Previous, int32 Depth, const FString &Terminal) {
  return Depth > MazeConfig().DeadEndDepth
             ? FDeadEndResult{State.RandomState, State.Cells, State.CellOrder,
                              Terminal}
             : [&]() {
                 const FString Cell = FormatDeadEndId(Branch, Depth);
                 FMazeGenerationState Candidate = State;
                 Candidate.Cells.Add(Cell, {});
                 Candidate.CellOrder.Add(Cell);
                 const FPassageResult Passage = OpenPassage(
                     Candidate.Cells, Candidate.RandomState, Previous, Cell);
                 return !Passage.bOpened
                            ? FDeadEndResult{Passage.State, State.Cells,
                                             State.CellOrder, Terminal}
                            : [&]() {
                                Candidate.RandomState = Passage.State;
                                Candidate.Cells = Passage.Cells;
                                return ExtendDeadEnd(
                                    Candidate, Branch, Cell,
                                    Depth + MazeConfig().Numbers.One, Cell);
                              }();
               }();
}

/**
 * User Story: As Maze generation, I need the authored number of distinct dead-end corridors accumulated recursively.
 * @fn inline FMazeGenerationState AddDeadEnds( const TArray<FString> &Anchors, int32 Index, FMazeGenerationState State)
 */
inline FMazeGenerationState AddDeadEnds(
    const TArray<FString> &Anchors, int32 Index,
    FMazeGenerationState State) {
  return Index >= MazeConfig().DeadEndCount || Index >= Anchors.Num()
             ? State
             : [&]() {
                 const FDeadEndResult Branch = ExtendDeadEnd(
                     State, Index + MazeConfig().Numbers.One, Anchors[Index],
                     MazeConfig().Numbers.One, MazeConfig().EmptyContext);
                 State.RandomState = Branch.State;
                 State.Cells = Branch.Cells;
                 State.CellOrder = Branch.CellOrder;
                 !Branch.Terminal.IsEmpty()
                     ? (State.DeadEnds.Add(Branch.Terminal), void())
                     : void();
                 return AddDeadEnds(Anchors, Index + MazeConfig().Numbers.One,
                                    MoveTemp(State));
               }();
}

/** User Story: As the shared Maze benchmark, I need one deterministic solvable topology generated from an explicit seed. @fn inline FMaze GenerateMaze(uint32 Seed) */
inline FMaze GenerateMaze(uint32 Seed) {
  const FIntegerPick Length = PickInteger(
      Seed, MazeConfig().MainPathMin, MazeConfig().MainPathMax);
  const TArray<FString> MainPath = BuildMainPath(
      Length.Value, MazeConfig().Numbers.Zero, {});
  FMazeGenerationState Initial;
  Initial.RandomState = Length.State;
  const FMazeGenerationState Connected = ConnectMainPath(
      MainPath, MazeConfig().Numbers.Zero,
      InitializeCells(MainPath, MazeConfig().Numbers.Zero, Initial));
  const TArray<FString> Interior = SelectInteriorPath(
      MainPath, MazeConfig().Numbers.One, {});
  const FShuffleResult Shuffled =
      ShuffleValues(Interior, Connected.RandomState);
  FMazeGenerationState BranchState = Connected;
  BranchState.RandomState = Shuffled.State;
  const FMazeGenerationState Complete = AddDeadEnds(
      Shuffled.Values, MazeConfig().Numbers.Zero, BranchState);
  return {Complete.Cells,
          Complete.CellOrder,
          MazeConfig().StartId,
          MazeConfig().ExitId,
          Complete.DeadEnds,
          MeasureMazeDistances(Complete.Cells, MazeConfig().ExitId),
          MainPath.Num() - MazeConfig().Numbers.One,
          Seed};
}

/** User Story: As an unseeded Maze run, I need a transient host seed selected only at the effect boundary. @fn inline uint32 CurrentMazeSeed() */
inline uint32 CurrentMazeSeed() {
  return static_cast<uint32>(FPlatformTime::Cycles64());
}

} // namespace MicroGame::Maze
