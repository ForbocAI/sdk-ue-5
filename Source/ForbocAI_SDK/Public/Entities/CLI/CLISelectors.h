#pragma once

#include "Core/fp.hpp"
#include "Entities/CLI/Bridge/CLIBridgeSelectors.h"
#include "Components/CLI/CLITypes.h"
#include "Entities/CLI/Ghost/CLIGhostSelectors.h"
#include "Entities/CLI/Memory/CLIMemorySelectors.h"
#include "Entities/CLI/NPC/CLINPCSelectors.h"
#include "Entities/CLI/Soul/CLISoulSelectors.h"

namespace ForbocAI {
namespace CLI {

namespace Bridge {
/** User Story: As a features cli consumer, I need to invoke select cli bridge through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FCLIBridgeState &selectCliBridge(const FCLIState &State) */
inline const FCLIBridgeState &selectCliBridge(const FCLIState &State) {
  return State.Bridge;
}
} // namespace Bridge

namespace Memory {
/** User Story: As a features cli consumer, I need to invoke select cli memory through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FCLIMemoryState &selectCliMemory(const FCLIState &State) */
inline const FCLIMemoryState &selectCliMemory(const FCLIState &State) {
  return State.Memory;
}
} // namespace Memory

namespace NPC {
/** User Story: As a features cli consumer, I need to invoke select cli npc through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FCLINPCState &selectCliNpc(const FCLIState &State) */
inline const FCLINPCState &selectCliNpc(const FCLIState &State) {
  return State.NPC;
}
} // namespace NPC

namespace Soul {
/** User Story: As a features CLI consumer, I need the Soul CLI state selected through a stable signature so the package root store remains authoritative. @fn inline const FCLISoulState &selectCliSoul(const FCLIState &State) */
inline const FCLISoulState &selectCliSoul(const FCLIState &State) {
  return State.Soul;
}
} // namespace Soul

/** User Story: As a features cli consumer, I need to invoke select cli command matrix through a stable signature so the features cli workflow remains explicit and composable. @fn inline const TArray<FCLICommandSpec> &selectCliCommandMatrix( const FCLIState &State) */
inline const TArray<FCLICommandSpec> &selectCliCommandMatrix(
    const FCLIState &State) {
  return State.Commands;
}

/** User Story: As a features cli consumer, I need to invoke select node cli surface through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FString &selectNodeCliSurface(const FCLIState &State) */
inline const FString &selectNodeCliSurface(const FCLIState &State) {
  return State.NodeSurface;
}

/** User Story: As a features cli consumer, I need to invoke select browser cli surface through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FString &selectBrowserCliSurface(const FCLIState &State) */
inline const FString &selectBrowserCliSurface(const FCLIState &State) {
  return State.BrowserSurface;
}

/** User Story: As a features cli consumer, I need to invoke select cli command roles through a stable signature so the features cli workflow remains explicit and composable. @fn inline const FCLICommandRoles &selectCliCommandRoles( const FCLIState &State) */
inline const FCLICommandRoles &selectCliCommandRoles(
    const FCLIState &State) {
  return State.CommandRoles;
}

/** User Story: As a features cli consumer, I need to invoke has surface through a stable signature so the features cli workflow remains explicit and composable. @fn inline bool HasSurface(const FCLICommandSpec &Command, const FString &Surface) */
inline bool HasSurface(const FCLICommandSpec &Command,
                       const FString &Surface) {
  return func::any_array<FString>(
      Command.Surfaces,
      [&Surface](const FString &Candidate) { return Candidate == Surface; });
}

/** User Story: As a features cli consumer, I need to invoke select cli command keys through a stable signature so the features cli workflow remains explicit and composable. @fn inline TArray<FString> selectCliCommandKeys(const FCLIState &State, const FString &Surface) */
inline TArray<FString> selectCliCommandKeys(const FCLIState &State,
                                            const FString &Surface) {
  return func::map_array<FCLICommandSpec, FString>(
      func::filter_array<FCLICommandSpec>(
          selectCliCommandMatrix(State),
          [&Surface](const FCLICommandSpec &Command) {
            return HasSurface(Command, Surface);
          }),
      [](const FCLICommandSpec &Command) { return Command.Key; });
}

/** User Story: As a features cli consumer, I need to invoke get cli command spec recursive through a stable signature so the features cli workflow remains explicit and composable. @fn inline func::Maybe<FCLICommandSpec> getCliCommandSpecRecursive( const FCLIState &State, const FString &Key, int32 Index) */
inline func::Maybe<FCLICommandSpec> getCliCommandSpecRecursive(
    const FCLIState &State, const FString &Key, int32 Index) {
  return Index >= State.Commands.Num()
             ? func::nothing<FCLICommandSpec>()
             : State.Commands[Index].Key == Key
                   ? func::just<FCLICommandSpec>(State.Commands[Index])
                   : getCliCommandSpecRecursive(
                         State, Key, Index + State.Parsing.NextIndexOffset);
}

/** User Story: As a features cli consumer, I need to invoke get cli command spec through a stable signature so the features cli workflow remains explicit and composable. @fn inline func::Maybe<FCLICommandSpec> getCliCommandSpec( const FCLIState &State, const FString &Key) */
inline func::Maybe<FCLICommandSpec> getCliCommandSpec(
    const FCLIState &State, const FString &Key) {
  return getCliCommandSpecRecursive(State, Key,
                                    State.Parsing.FirstTokenIndex);
}

/** User Story: As a features cli consumer, I need to invoke is valid command key through a stable signature so the features cli workflow remains explicit and composable. @fn inline bool isValidCommandKey(const FCLIState &State, const FString &Key) */
inline bool isValidCommandKey(const FCLIState &State,
                              const FString &Key) {
  const func::Maybe<FCLICommandSpec> Command = getCliCommandSpec(State, Key);
  return Command.hasValue && HasSurface(Command.value, State.NodeSurface);
}

/** User Story: As a features cli consumer, I need to invoke resolve node cli command key recursive through a stable signature so the features cli workflow remains explicit and composable. @fn inline FString resolveNodeCliCommandKeyRecursive( const FCLIState &State, const FString &Group, const FString &Subcommand, int32 Index) */
inline FString resolveNodeCliCommandKeyRecursive(
    const FCLIState &State, const FString &Group,
    const FString &Subcommand, int32 Index) {
  return Index >= State.Commands.Num()
             ? FString()
             : (HasSurface(State.Commands[Index], State.NodeSurface) &&
                State.Commands[Index].Group == Group &&
                State.Commands[Index].Subcommand == Subcommand)
                   ? State.Commands[Index].Key
                   : resolveNodeCliCommandKeyRecursive(
                         State, Group, Subcommand,
                         Index + State.Parsing.NextIndexOffset);
}

/** User Story: As a features cli consumer, I need to invoke resolve node cli command key through a stable signature so the features cli workflow remains explicit and composable. @fn inline FString resolveNodeCliCommandKey(const FCLIState &State, const FString &Group, const FString &Subcommand) */
inline FString resolveNodeCliCommandKey(const FCLIState &State,
                                        const FString &Group,
                                        const FString &Subcommand) {
  return resolveNodeCliCommandKeyRecursive(
      State, Group, Subcommand, State.Parsing.FirstTokenIndex);
}

/** User Story: As a features cli consumer, I need to invoke drop args recursive through a stable signature so the features cli workflow remains explicit and composable. @fn inline TArray<FString> DropArgsRecursive( const FCLIState &State, const TArray<FString> &Input, int32 DropCount, int32 Index, TArray<FString> Output) */
inline TArray<FString> DropArgsRecursive(
    const FCLIState &State, const TArray<FString> &Input,
    int32 DropCount, int32 Index, TArray<FString> Output) {
  return Index >= Input.Num()
             ? Output
             : Index < DropCount
                   ? DropArgsRecursive(
                         State, Input, DropCount,
                         Index + State.Parsing.NextIndexOffset,
                         MoveTemp(Output))
                   : (Output.Add(Input[Index]),
                      DropArgsRecursive(
                          State, Input, DropCount,
                          Index + State.Parsing.NextIndexOffset,
                          MoveTemp(Output)));
}

/** User Story: As a features cli consumer, I need to invoke drop args through a stable signature so the features cli workflow remains explicit and composable. @fn inline TArray<FString> DropArgs(const FCLIState &State, const TArray<FString> &Input, int32 DropCount) */
inline TArray<FString> DropArgs(const FCLIState &State,
                                const TArray<FString> &Input,
                                int32 DropCount) {
  return DropArgsRecursive(State, Input, DropCount,
                           State.Parsing.FirstTokenIndex,
                           TArray<FString>());
}

/** User Story: As a features cli consumer, I need to invoke parsed through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCommandParseResult Parsed(const FString &CommandKey, const TArray<FString> &Args) */
inline FCommandParseResult Parsed(const FString &CommandKey,
                                  const TArray<FString> &Args) {
  FCommandParseResult Result;
  Result.bMatched = true;
  Result.CommandKey = CommandKey;
  Result.Args = Args;
  return Result;
}

/** User Story: As a features cli consumer, I need to invoke not parsed through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCommandParseResult NotParsed() */
inline FCommandParseResult NotParsed() {
  FCommandParseResult Result;
  Result.bMatched = false;
  return Result;
}

/** User Story: As a features cli consumer, I need to invoke resolve node cli command through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCommandParseResult resolveNodeCliCommand( const FCLIState &State, const TArray<FString> &Tokens) */
inline FCommandParseResult resolveNodeCliCommand(
    const FCLIState &State, const TArray<FString> &Tokens) {
  const FCLIParsingSettings &Parsing = State.Parsing;
  const FString DirectKey = Tokens.Num() > Parsing.FirstTokenIndex
                                ? Tokens[Parsing.FirstTokenIndex]
                                : FString();
  const FString GroupKey =
      Tokens.Num() > Parsing.SecondTokenIndex
          ? resolveNodeCliCommandKey(
                State, Tokens[Parsing.FirstTokenIndex],
                Tokens[Parsing.SecondTokenIndex])
          : FString();

  return Tokens.Num() <= Parsing.MinimumTokenCount
             ? NotParsed()
             : isValidCommandKey(State, DirectKey)
                   ? Parsed(DirectKey,
                            DropArgs(State, Tokens,
                                     Parsing.DirectDropCount))
                   : !GroupKey.IsEmpty()
                         ? Parsed(GroupKey,
                                  DropArgs(State, Tokens,
                                           Parsing.SubcommandDropCount))
                         : NotParsed();
}

} // namespace CLI
} // namespace ForbocAI
