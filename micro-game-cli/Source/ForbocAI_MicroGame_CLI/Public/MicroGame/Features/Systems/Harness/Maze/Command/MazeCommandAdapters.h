#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"

namespace MicroGame::Maze {

/** User Story: As either Maze transport, I need attribute generation composed from the injected command template. @fn inline FString FormatMazeGenerate(const FMazeCommands &Commands, const FString &Attribute, const FString &Context) */
inline FString FormatMazeGenerate(const FMazeCommands &Commands,
                                  const FString &Attribute,
                                  const FString &Context) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Commands.Generate,
      {{Data.Tokens.Attribute, Attribute},
       {Data.Tokens.Context,
        Context.IsEmpty()
            ? Data.EmptyContext
            : Data.ContextQuote + Context + Data.ContextQuote}});
}

/** User Story: As either Maze transport, I need actor creation composed from the injected command template. @fn inline FString FormatMazeCreate(const FMazeCommands &Commands, const FString &Persona) */
inline FString FormatMazeCreate(const FMazeCommands &Commands,
                                const FString &Persona) {
  return ApplyMazeToken(Commands.Create, MazeConfig().Tokens.Persona,
                        Persona);
}

/** User Story: As either Maze transport, I need identity update composed from the injected command template. @fn inline FString FormatMazeIdentityUpdate(const FMazeCommands &Commands, const FString &Id, const FString &Name, const FString &Role) */
inline FString FormatMazeIdentityUpdate(const FMazeCommands &Commands,
                                        const FString &Id,
                                        const FString &Name,
                                        const FString &Role) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Commands.IdentityUpdate,
      {{Data.Tokens.Id, Id},
       {Data.Tokens.Name, Name},
       {Data.Tokens.Role, Role}});
}

/** User Story: As either Maze transport, I need actor recall composed from the injected command template. @fn inline FString FormatMazeRecall(const FMazeCommands &Commands, const FString &Id) */
inline FString FormatMazeRecall(const FMazeCommands &Commands,
                                const FString &Id) {
  return ApplyMazeToken(Commands.Recall, MazeConfig().Tokens.Id, Id);
}

/**
 * User Story: As either Maze transport, I need constrained decision input composed from the injected command template.
 * @fn inline FString FormatMazeDecide( const FMazeCommands &Commands, const FString &Id, const FString &Line, const TArray<FString> &Options, const TArray<FString> &Visited, const TArray<FString> &Avoid)
 */
inline FString FormatMazeDecide(
    const FMazeCommands &Commands, const FString &Id, const FString &Line,
    const TArray<FString> &Options, const TArray<FString> &Visited,
    const TArray<FString> &Avoid) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Commands.Decide,
      {{Data.Tokens.Id, Id},
       {Data.Tokens.Line, Line},
       {Data.Tokens.Options, FString::Join(Options, *Data.OptionSeparator)},
       {Data.Tokens.Visited, FString::Join(Visited, *Data.OptionSeparator)},
       {Data.Tokens.Avoid, FString::Join(Avoid, *Data.OptionSeparator)}});
}

/** User Story: As either Maze transport, I need vector memory composed from the injected command template. @fn inline FString FormatMazeMemoryStore(const FMazeCommands &Commands, const FString &Id, const FString &Memory) */
inline FString FormatMazeMemoryStore(const FMazeCommands &Commands,
                                     const FString &Id,
                                     const FString &Memory) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Commands.MemoryStore,
      {{Data.Tokens.Id, Id}, {Data.Tokens.Memory, Memory}});
}

/** User Story: As either Maze transport, I need typed vector memory composed from the injected command template. @fn inline FString FormatMazeTypedMemoryStore(const FMazeCommands &Commands, const FString &Id, const FString &Memory, const FString &Type) */
inline FString FormatMazeTypedMemoryStore(const FMazeCommands &Commands,
                                          const FString &Id,
                                          const FString &Memory,
                                          const FString &Type) {
  const FMazeConfig &Data = MazeConfig();
  return ApplyMazeValues(
      Commands.TypedMemoryStore,
      {{Data.Tokens.Id, Id},
       {Data.Tokens.Memory, Memory},
       {Data.Tokens.Type, Type}});
}

} // namespace MicroGame::Maze
