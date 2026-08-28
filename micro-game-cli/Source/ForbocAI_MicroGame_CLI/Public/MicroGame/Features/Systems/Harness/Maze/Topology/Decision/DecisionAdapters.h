#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"
#include "MicroGame/Features/Components/Harness/Maze/Topology/TopologyTypes.h"

namespace MicroGame::Maze {

/** User Story: As Maze decisions, I need open passages selected in authored direction order. @fn inline TArray<FString> OpenDirections(const FMaze &Maze, const FString &Cell) */
inline TArray<FString> OpenDirections(const FMaze &Maze,
                                      const FString &Cell) {
  const TMap<FString, FString> *Passages = Maze.Cells.Find(Cell);
  return Passages == nullptr
             ? TArray<FString>()
             : func::filter_array<FString>(
                   MazeConfig().Directions,
                   [Passages](const FString &Direction) {
                     return Passages->Contains(Direction);
                   });
}

/** User Story: As Maze scoring, I need every direction that strictly reduces shortest distance to the exit. @fn inline TArray<FString> OptimalDirections(const FMaze &Maze, const FString &Cell) */
inline TArray<FString> OptimalDirections(const FMaze &Maze,
                                         const FString &Cell) {
  return func::filter_array<FString>(
      OpenDirections(Maze, Cell),
      [&Maze, &Cell](const FString &Direction) {
        const FString Neighbor =
            Maze.Cells.FindChecked(Cell).FindChecked(Direction);
        return Maze.Distances.FindChecked(Neighbor) ==
               Maze.Distances.FindChecked(Cell) - MazeConfig().Numbers.One;
      });
}

/**
 * User Story: As Maze ward diagnostics, I need the legal, visited, immediate-dead-end, narrowed, and optimal directions derived once.
 * @fn inline FWardOutcome MeasureWardOutcome( const FMaze &Maze, const FString &Cell, const TArray<FString> &Visited)
 */
inline FWardOutcome MeasureWardOutcome(
    const FMaze &Maze, const FString &Cell,
    const TArray<FString> &Visited) {
  FWardOutcome Outcome;
  Outcome.Options = OpenDirections(Maze, Cell);
  Outcome.VisitedDirections = func::filter_array<FString>(
      Outcome.Options, [&Maze, &Cell, &Visited](const FString &Direction) {
        return Visited.Contains(
            Maze.Cells.FindChecked(Cell).FindChecked(Direction));
      });
  Outcome.AvoidDirections = func::filter_array<FString>(
      Outcome.Options, [&Maze, &Cell](const FString &Direction) {
        return Maze.DeadEnds.Contains(
            Maze.Cells.FindChecked(Cell).FindChecked(Direction));
      });
  const TArray<FString> Narrowed = func::filter_array<FString>(
      Outcome.Options, [&Outcome](const FString &Direction) {
        return !Outcome.VisitedDirections.Contains(Direction) &&
               !Outcome.AvoidDirections.Contains(Direction);
      });
  Outcome.WardDirections =
      Narrowed.IsEmpty() ? Outcome.Options : Narrowed;
  Outcome.bRealChoice =
      Outcome.WardDirections.Num() >= MazeConfig().MinRealChoice;
  Outcome.OptimalDirections = OptimalDirections(Maze, Cell);
  return Outcome;
}

/**
 * User Story: As free-text Maze movement, I need the earliest named open direction selected without guessing.
 * @fn inline func::Maybe<FString> ParseDirection( const FString &Dialogue, const TArray<FString> &Options)
 */
inline func::Maybe<FString> ParseDirection(
    const FString &Dialogue, const TArray<FString> &Options) {
  const FString Text = Dialogue.ToLower();
  const TArray<FDirectionHit> Hits = func::filter_array<FDirectionHit>(
      func::map_array<FString, FDirectionHit>(
          Options, [&Text](const FString &Direction) {
            return FDirectionHit{Direction,
                                 Text.Find(Direction.ToLower())};
          }),
      [](const FDirectionHit &Hit) {
        return Hit.Index >= MazeConfig().Numbers.Zero;
      });
  return Hits.IsEmpty()
             ? func::nothing<FString>()
             : func::just(func::fold_array<FDirectionHit, FDirectionHit>(
                              Hits, Hits[MazeConfig().Numbers.Zero],
                              [](const FDirectionHit &Selected,
                                 const FDirectionHit &Candidate) {
                                return Candidate.Index < Selected.Index
                                           ? Candidate
                                           : Selected;
                              })
                              .Direction);
}

/** User Story: As grounded Maze decisions, I need the current cell, passages, and move count composed into one authored prompt. @fn inline FString FormatTurnPrompt(const FString &Cell, const TArray<FString> &Options, int32 Moves) */
inline FString FormatTurnPrompt(const FString &Cell,
                                const TArray<FString> &Options,
                                int32 Moves) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.TurnPrompt,
      {{Data.Tokens.Cell, Cell},
       {Data.Tokens.Options,
        FString::Join(Options, *Data.DisplayOptionSeparator)},
       {Data.Tokens.Moves, FString::FromInt(Moves)}});
}

} // namespace MicroGame::Maze
