#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::Maze {

/** User Story: As Maze formatting, I need one authored token replaced without regular-expression behavior. @fn inline FString ApplyMazeToken(const FString &Template, const FString &Token, const FString &Value) */
inline FString ApplyMazeToken(const FString &Template, const FString &Token,
                              const FString &Value) {
  return Template.Replace(*Token, *Value);
}

/**
 * User Story: As Maze formatting, I need token substitutions folded over one authored template.
 * @fn inline FString ApplyMazeValues( const FString &Template, const TArray<TPair<FString, FString>> &Values)
 */
inline FString ApplyMazeValues(
    const FString &Template,
    const TArray<TPair<FString, FString>> &Values) {
  return func::fold_array<TPair<FString, FString>, FString>(
      Values, Template,
      [](const FString &Current,
         const TPair<FString, FString> &Value) {
        return ApplyMazeToken(Current, Value.Key, Value.Value);
      });
}

/** User Story: As Maze topology generation, I need main-path identifiers composed from authored tokens. @fn inline FString FormatCellId(int32 Index) */
inline FString FormatCellId(int32 Index) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.CellIdFormat,
      {{Data.Tokens.Prefix, Data.CellPrefix},
       {Data.Tokens.Index, FString::FromInt(Index)}});
}

/** User Story: As Maze topology generation, I need dead-end identifiers composed from authored branch and depth tokens. @fn inline FString FormatDeadEndId(int32 Index, int32 Depth) */
inline FString FormatDeadEndId(int32 Index, int32 Depth) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Data.DeadEndIdFormat,
      {{Data.Tokens.Prefix, Data.DeadEndPrefix},
       {Data.Tokens.Index, FString::FromInt(Index)},
       {Data.Tokens.Depth, FString::FromInt(Depth)}});
}

} // namespace MicroGame::Maze
