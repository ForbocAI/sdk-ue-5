#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Topology/Random/RandomAdapters.h"

namespace MicroGame::Maze {

/**
 * User Story: As reciprocal Maze topology, I need the first shuffled free direction selected without mutating the source cells.
 * @fn inline FString SelectPassageDirection( const TMap<FString, TMap<FString, FString>> &Cells, const FString &From, const FString &To, const TArray<FString> &Directions, int32 Index)
 */
inline FString SelectPassageDirection(
    const TMap<FString, TMap<FString, FString>> &Cells,
    const FString &From, const FString &To,
    const TArray<FString> &Directions, int32 Index) {
  const FMazeConfig &Data = MazeConfig();
  return Index >= Directions.Num()
             ? Data.EmptyContext
             : !Cells.FindChecked(From).Contains(Directions[Index]) &&
                       !Cells.FindChecked(To).Contains(
                           Data.Opposites.FindChecked(Directions[Index]))
                   ? Directions[Index]
                   : SelectPassageDirection(
                         Cells, From, To, Directions,
                         Index + Data.Numbers.One);
}

/**
 * User Story: As deterministic Maze generation, I need one reciprocal passage attempt returned with its next random state and copied topology.
 * @fn inline FPassageResult OpenPassage( const TMap<FString, TMap<FString, FString>> &Cells, uint32 State, const FString &From, const FString &To)
 */
inline FPassageResult OpenPassage(
    const TMap<FString, TMap<FString, FString>> &Cells, uint32 State,
    const FString &From, const FString &To) {
  const FShuffleResult Shuffled = ShuffleValues(MazeConfig().Directions, State);
  const FString Direction = SelectPassageDirection(
      Cells, From, To, Shuffled.Values, MazeConfig().Numbers.Zero);
  return Direction.IsEmpty()
             ? FPassageResult{Shuffled.State, Cells, false}
             : [&]() {
                 TMap<FString, TMap<FString, FString>> Next = Cells;
                 Next.FindChecked(From).Add(Direction, To);
                 Next.FindChecked(To).Add(
                     MazeConfig().Opposites.FindChecked(Direction), From);
                 return FPassageResult{Shuffled.State, MoveTemp(Next), true};
               }();
}

} // namespace MicroGame::Maze
