#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"
#include "MicroGame/Features/Components/Harness/Maze/Topology/TopologyTypes.h"

namespace MicroGame::Maze {

/**
 * User Story: As reproducible Maze evidence, I need one cell's passages rendered in authored direction order.
 * @fn inline TArray<FString> FormatMazeEdges( const FMaze &Maze, const FString &Cell, int32 Index, TArray<FString> Lines)
 */
inline TArray<FString> FormatMazeEdges(
    const FMaze &Maze, const FString &Cell, int32 Index,
    TArray<FString> Lines) {
  return Index >= MazeConfig().Directions.Num()
             ? Lines
             : [&]() {
                 const FString Direction = MazeConfig().Directions[Index];
                 const FString *Neighbor =
                     Maze.Cells.FindChecked(Cell).Find(Direction);
                 Neighbor != nullptr
                     ? Lines.Add(ApplyMazeValues(
                           MazeConfig().EdgeFormat,
                           {{MazeConfig().Tokens.Direction, Direction},
                            {MazeConfig().Tokens.Neighbor, *Neighbor}}))
                     : int32{};
                 return FormatMazeEdges(
                     Maze, Cell, Index + MazeConfig().Numbers.One,
                     MoveTemp(Lines));
               }();
}

/**
 * User Story: As reproducible Maze evidence, I need each cell and its passages accumulated in stable generation order.
 * @fn inline TArray<FString> FormatMazeLayoutCells( const FMaze &Maze, int32 Index, TArray<FString> Lines)
 */
inline TArray<FString> FormatMazeLayoutCells(
    const FMaze &Maze, int32 Index, TArray<FString> Lines) {
  return Index >= Maze.CellOrder.Num()
             ? Lines
             : [&]() {
                 const FString Cell = Maze.CellOrder[Index];
                 const FString Edges = FString::Join(
                     FormatMazeEdges(Maze, Cell, MazeConfig().Numbers.Zero,
                                     {}),
                     *MazeConfig().DisplayOptionSeparator);
                 Lines.Add(ApplyMazeValues(
                     MazeConfig().LayoutEntry,
                     {{MazeConfig().Tokens.Cell, Cell},
                      {MazeConfig().Tokens.Edges, Edges}}));
                 return FormatMazeLayoutCells(
                     Maze, Index + MazeConfig().Numbers.One,
                     MoveTemp(Lines));
               }();
}

/** User Story: As reproducible Maze evidence, I need the topology header and stable cell layout returned together. @fn inline TArray<FString> FormatMazeLayout(const FMaze &Maze) */
inline TArray<FString> FormatMazeLayout(const FMaze &Maze) {
  return FormatMazeLayoutCells(
      Maze, MazeConfig().Numbers.Zero, {MazeConfig().LayoutHeader});
}

} // namespace MicroGame::Maze
