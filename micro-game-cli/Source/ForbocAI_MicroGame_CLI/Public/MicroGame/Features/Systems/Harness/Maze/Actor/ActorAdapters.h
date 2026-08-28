#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"

namespace MicroGame::Maze {

/** User Story: As generated Maze personas, I need prior values threaded through the authored context separator. @fn inline FString ExtendMazeContext(const FString &Context, const FString &Value) */
inline FString ExtendMazeContext(const FString &Context,
                                 const FString &Value) {
  return Context.IsEmpty()
             ? Value
             : Context + MazeConfig().ContextSeparator + Value;
}

/** User Story: As Maze actor creation, I need non-empty generated attributes joined through the authored persona separator. @fn inline FString JoinMazePersona(const TArray<FString> &Values) */
inline FString JoinMazePersona(const TArray<FString> &Values) {
  return FString::Join(
      func::filter_array<FString>(Values, [](const FString &Value) {
        return !Value.IsEmpty();
      }),
      *MazeConfig().PersonaSeparator);
}

/** User Story: As Maze identity setup, I need the role selected by authored attribute name instead of positional coupling. @fn inline FString SelectMazeRole(const TArray<FString> &Values) */
inline FString SelectMazeRole(const TArray<FString> &Values) {
  const int32 Index =
      MazeConfig().PersonaAttributes.IndexOfByKey(MazeConfig().RoleAttribute);
  return Values.IsValidIndex(Index) ? Values[Index]
                                    : MazeConfig().DialogueFallback;
}

/** User Story: As Maze evidence, I need the generated name and role composed through authored display formatting. @fn inline FString FormatMazeDisplayName(const FString &Name, const FString &Role) */
inline FString FormatMazeDisplayName(const FString &Name,
                                     const FString &Role) {
  return Role.TrimStartAndEnd().IsEmpty()
             ? Name
             : ApplyMazeValues(
                   MazeConfig().DisplayNameFormat,
                   {{MazeConfig().Tokens.Name, Name},
                    {MazeConfig().Tokens.Role, Role}});
}

} // namespace MicroGame::Maze
