#pragma once

#include "Features/CLI/CLITypes.h"
#include "Core/fp.hpp"
#include "Features/Data/DataAdapters.h"

namespace ForbocAI {
namespace CLI {

inline FString ReadSubcommand(const TSharedPtr<FJsonObject> &Command) {
  FString Subcommand;
  Command->TryGetStringField(TEXT("subcommand"), Subcommand);
  return Subcommand;
}

inline bool HasNodeSurface(const TSharedPtr<FJsonObject> &Command) {
  const TArray<TSharedPtr<FJsonValue>> &Surfaces =
      Command->GetArrayField(TEXT("surfaces"));
  return func::any_array<TSharedPtr<FJsonValue>>(
      Surfaces, [](const TSharedPtr<FJsonValue> &Surface) {
        return Surface.IsValid() && Surface->AsString() == TEXT("node");
      });
}

inline TArray<FCommandRoute> ReadNodeParityRoutes() {
  const TSharedRef<FJsonObject> Root =
      SDKData::LoadObject(TEXT("cli/commands.json"));
  const TSharedPtr<FJsonObject> Commands =
      Root->GetObjectField(TEXT("commands"));
  const TArray<FJsonObject::FStringType> StoredKeys =
      func::map_keys(Commands->Values);
  const TArray<FString> Keys = func::map_array(
      StoredKeys, [](const FJsonObject::FStringType &Key) {
        return FString(Key.ToView());
      });
  const TArray<FString> NodeKeys = func::filter_array<FString>(
      Keys, [Commands](const FString &Key) {
        return HasNodeSurface(Commands->GetObjectField(Key));
      });
  return func::map_array<FString, FCommandRoute>(
      NodeKeys, [Commands](const FString &Key) {
        const TSharedPtr<FJsonObject> Command =
            Commands->GetObjectField(Key);
        return FCommandRoute{Key, Command->GetStringField(TEXT("group")),
                             ReadSubcommand(Command)};
      });
}

inline const TArray<FCommandRoute> &NodeParityRoutes() {
  static const TArray<FCommandRoute> Routes = ReadNodeParityRoutes();
  return Routes;
}

inline bool ContainsRouteKeyRecursive(const TArray<FCommandRoute> &Routes,
                                      const FString &Key, int32 Index) {
  return Index >= Routes.Num()
             ? false
             : Routes[Index].Key == Key
                   ? true
                   : ContainsRouteKeyRecursive(Routes, Key, Index + 1);
}

inline bool IsNodeParityCommandKey(const FString &Key) {
  return ContainsRouteKeyRecursive(NodeParityRoutes(), Key, 0);
}

inline bool IsValidCommandKey(const FString &Key) {
  return IsNodeParityCommandKey(Key);
}

inline FString RouteKeyForGroupRecursive(const TArray<FCommandRoute> &Routes,
                                         const FString &Group,
                                         const FString &Subcommand,
                                         int32 Index) {
  return Index >= Routes.Num()
             ? FString(TEXT(""))
             : (Routes[Index].Group == Group &&
                Routes[Index].Subcommand == Subcommand)
                   ? Routes[Index].Key
                   : RouteKeyForGroupRecursive(Routes, Group, Subcommand,
                                               Index + 1);
}

inline FString RouteKeyForGroup(const FString &Group,
                                const FString &Subcommand) {
  return RouteKeyForGroupRecursive(NodeParityRoutes(), Group, Subcommand, 0);
}

inline TArray<FString> DropArgsRecursive(const TArray<FString> &Input,
                                         int32 DropCount, int32 Index,
                                         TArray<FString> Output) {
  return Index >= Input.Num()
             ? Output
             : Index < DropCount
                   ? DropArgsRecursive(Input, DropCount, Index + 1,
                                       MoveTemp(Output))
                   : (Output.Add(Input[Index]),
                      DropArgsRecursive(Input, DropCount, Index + 1,
                                        MoveTemp(Output)));
}

inline TArray<FString> DropArgs(const TArray<FString> &Input,
                                int32 DropCount) {
  return DropArgsRecursive(Input, DropCount, 0, TArray<FString>());
}

inline FCommandParseResult Parsed(const FString &CommandKey,
                                  const TArray<FString> &Args) {
  FCommandParseResult Result;
  Result.bMatched = true;
  Result.CommandKey = CommandKey;
  Result.Args = Args;
  return Result;
}

inline FCommandParseResult NotParsed() {
  FCommandParseResult Result;
  Result.bMatched = false;
  return Result;
}

inline FCommandParseResult ParseCommandTokens(const TArray<FString> &Tokens) {
  const FString DirectKey = Tokens.Num() > 0 ? Tokens[0] : FString(TEXT(""));
  const FString GroupKey =
      Tokens.Num() > 1 ? RouteKeyForGroup(Tokens[0], Tokens[1])
                       : FString(TEXT(""));

  return Tokens.Num() <= 0
             ? NotParsed()
             : IsValidCommandKey(DirectKey)
                   ? Parsed(DirectKey, DropArgs(Tokens, 1))
                   : !GroupKey.IsEmpty()
                         ? Parsed(GroupKey, DropArgs(Tokens, 2))
                         : NotParsed();
}

} // namespace CLI
} // namespace ForbocAI
