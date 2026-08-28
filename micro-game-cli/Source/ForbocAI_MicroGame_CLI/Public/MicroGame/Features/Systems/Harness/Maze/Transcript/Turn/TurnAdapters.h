#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze debug evidence, I need the exact decision stimulus rendered for each turn. @fn inline FString FormatMazeQuery(int32 Turn, const FString &Cell, const FString &Query) */
inline FString FormatMazeQuery(int32 Turn, const FString &Cell,
                               const FString &Query) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.QueryFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.Cell, Cell},
       {Data.Tokens.Query, Query}});
}

/** User Story: As Maze debug evidence, I need legal, visited, avoided, and narrowed choices rendered from one ward outcome. @fn inline FString FormatMazeWard(int32 Turn, const FString &Cell, const FWardOutcome &Ward) */
inline FString FormatMazeWard(int32 Turn, const FString &Cell,
                              const FWardOutcome &Ward) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.WardFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.Cell, Cell},
       {Data.Tokens.Legal,
        FString::Join(Ward.Options, *Data.DisplayOptionSeparator)},
       {Data.Tokens.Visited,
        FString::Join(Ward.VisitedDirections,
                      *Data.DisplayOptionSeparator)},
       {Data.Tokens.Avoid,
        FString::Join(Ward.AvoidDirections,
                      *Data.DisplayOptionSeparator)},
       {Data.Tokens.Narrowed,
        FString::Join(Ward.WardDirections,
                      *Data.DisplayOptionSeparator)}});
}

/** User Story: As Maze analytical evidence, I need the API-owned thought result rendered beside its decision turn. @fn inline FString FormatMazeThought(int32 Turn, const FString &Cell, const FString &Thought) */
inline FString FormatMazeThought(int32 Turn, const FString &Cell,
                                 const FString &Thought) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.ThoughtFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.Cell, Cell},
       {Data.Tokens.Thought, Thought}});
}

/** User Story: As Maze analytical evidence, I need the API-owned reasoning result rendered beside its decision turn. @fn inline FString FormatMazeReason(int32 Turn, const FString &Cell, const FString &Reason) */
inline FString FormatMazeReason(int32 Turn, const FString &Cell,
                                const FString &Reason) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.ReasonFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.Cell, Cell},
       {Data.Tokens.Reason, Reason}});
}

/** User Story: As Maze movement evidence, I need a parsed move and dead-end status rendered from authored prose. @fn inline FString FormatMazeMoved(int32 Turn, const FString &From, const FString &Direction, const FString &To, bool bDeadEnd) */
inline FString FormatMazeMoved(int32 Turn, const FString &From,
                               const FString &Direction,
                               const FString &To, bool bDeadEnd) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.MovedFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.From, From},
       {Data.Tokens.Direction, Direction},
       {Data.Tokens.To, To},
       {Data.Tokens.Note, bDeadEnd ? Data.DeadEndNote : Data.EmptyContext}});
}

/** User Story: As Maze movement evidence, I need an unparsed response recorded without inventing a direction. @fn inline FString FormatMazeUnparsed(int32 Turn, const FString &From, const FString &Dialogue) */
inline FString FormatMazeUnparsed(int32 Turn, const FString &From,
                                  const FString &Dialogue) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.UnparsedFormat,
      {{Data.Tokens.Turn, FString::FromInt(Turn)},
       {Data.Tokens.From, From},
       {Data.Tokens.Dialogue, Dialogue}});
}

} // namespace MicroGame::Maze
