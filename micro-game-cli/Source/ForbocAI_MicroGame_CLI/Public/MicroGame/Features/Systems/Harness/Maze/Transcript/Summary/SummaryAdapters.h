#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze result quality, I need genuine-junction optimal picks summarized without claiming perfection when no choice remained. @fn inline FString FormatMazeDecisionQuality(int32 Optimal, int32 Choices) */
inline FString FormatMazeDecisionQuality(int32 Optimal, int32 Choices) {
  const FMazeConfig &Data = MazeConfig();
  return Choices <= Data.Numbers.Zero
             ? Data.NoRealChoiceNote
             : ApplyMazeValues(
                   Data.DecisionQualityFormat,
                   {{Data.Tokens.Optimal, FString::FromInt(Optimal)},
                    {Data.Tokens.Choices, FString::FromInt(Choices)}});
}

/** User Story: As solved Maze evidence, I need turns, optimal distance, and encountered terminal traps rendered together. @fn inline FString FormatMazeSolved(int32 Turns, int32 Optimal, int32 DeadEnds) */
inline FString FormatMazeSolved(int32 Turns, int32 Optimal,
                                int32 DeadEnds) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.SolvedFormat,
      {{Data.Tokens.Turns, FString::FromInt(Turns)},
       {Data.Tokens.Optimal, FString::FromInt(Optimal)},
       {Data.Tokens.DeadEnds, FString::FromInt(DeadEnds)}});
}

/** User Story: As unfinished Maze evidence, I need turns, terminal cell, trap count, and optimal distance rendered together. @fn inline FString FormatMazeFailed(int32 Turns, const FString &Cell, int32 DeadEnds, int32 Optimal) */
inline FString FormatMazeFailed(int32 Turns, const FString &Cell,
                                int32 DeadEnds, int32 Optimal) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.FailedFormat,
      {{Data.Tokens.Turns, FString::FromInt(Turns)},
       {Data.Tokens.Cell, Cell},
       {Data.Tokens.DeadEnds, FString::FromInt(DeadEnds)},
       {Data.Tokens.Optimal, FString::FromInt(Optimal)}});
}

/** User Story: As reproducible Maze evidence, I need explorer identity and topology seed rendered together. @fn inline FString FormatMazeExplorer(const FString &Name, uint32 Seed) */
inline FString FormatMazeExplorer(const FString &Name, uint32 Seed) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.ExplorerFormat,
      {{Data.Tokens.Name, Name},
       {Data.Tokens.Seed, LexToString(Seed)}});
}

/** User Story: As Maze headline evidence, I need turns, optimal length, trap count, and completion rendered from one outcome. @fn inline FString FormatMazeRunSummary(int32 Turns, int32 Optimal, int32 DeadEnds, bool bSolved) */
inline FString FormatMazeRunSummary(int32 Turns, int32 Optimal,
                                    int32 DeadEnds, bool bSolved) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.RunSummaryFormat,
      {{Data.Tokens.Turns, FString::FromInt(Turns)},
       {Data.Tokens.Optimal, FString::FromInt(Optimal)},
       {Data.Tokens.DeadEnds, FString::FromInt(DeadEnds)},
       {Data.Tokens.Solved, LexToString(bSolved)}});
}

} // namespace MicroGame::Maze
