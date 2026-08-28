#pragma once

#include "MicroGame/Features/Components/Harness/Maze/Topology/TopologyTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::Maze {

/**
 * User Story: As shortest-path scoring, I need unseen neighbors accumulated into one immutable breadth-first state.
 * @fn inline FBreadthFirstState ExpandNeighbors( const TArray<FString> &Neighbors, int32 Index, const FString &Cell, const FBreadthFirstState &State)
 */
inline FBreadthFirstState ExpandNeighbors(
    const TArray<FString> &Neighbors, int32 Index, const FString &Cell,
    const FBreadthFirstState &State) {
  return Index >= Neighbors.Num()
             ? State
             : State.Distances.Contains(Neighbors[Index])
                   ? ExpandNeighbors(Neighbors, Index + MazeConfig().Numbers.One,
                                     Cell, State)
                   : [&]() {
                       FBreadthFirstState Next = State;
                       Next.Frontier.Add(Neighbors[Index]);
                       Next.Distances.Add(
                           Neighbors[Index],
                           State.Distances.FindChecked(Cell) +
                               MazeConfig().Numbers.One);
                       return ExpandNeighbors(
                           Neighbors, Index + MazeConfig().Numbers.One, Cell,
                           Next);
                     }();
}

/**
 * User Story: As Maze ground truth, I need breadth-first traversal expressed as recursive state transitions.
 * @fn inline TMap<FString, int32> TraverseDistances( const TMap<FString, TMap<FString, FString>> &Cells, const FBreadthFirstState &State)
 */
inline TMap<FString, int32> TraverseDistances(
    const TMap<FString, TMap<FString, FString>> &Cells,
    const FBreadthFirstState &State) {
  return State.Frontier.IsEmpty()
             ? State.Distances
             : [&]() {
                 FBreadthFirstState Next = State;
                 const FString Cell = Next.Frontier[MazeConfig().Numbers.Zero];
                 Next.Frontier.RemoveAt(MazeConfig().Numbers.Zero);
                 TArray<FString> Neighbors;
                 Cells.FindChecked(Cell).GenerateValueArray(Neighbors);
                 return TraverseDistances(
                     Cells, ExpandNeighbors(Neighbors, MazeConfig().Numbers.Zero,
                                            Cell, Next));
               }();
}

/**
 * User Story: As Maze decision scoring, I need shortest distances from every cell to the authored exit.
 * @fn inline TMap<FString, int32> MeasureMazeDistances( const TMap<FString, TMap<FString, FString>> &Cells, const FString &Exit)
 */
inline TMap<FString, int32> MeasureMazeDistances(
    const TMap<FString, TMap<FString, FString>> &Cells,
    const FString &Exit) {
  FBreadthFirstState Initial;
  Initial.Frontier.Add(Exit);
  Initial.Distances.Add(Exit, MazeConfig().Numbers.Zero);
  return TraverseDistances(Cells, Initial);
}

} // namespace MicroGame::Maze
