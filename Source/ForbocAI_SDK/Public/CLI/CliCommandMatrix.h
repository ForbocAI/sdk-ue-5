#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace CLI {

struct FCommandRoute {
  FString Key;
  FString Group;
  FString Subcommand;
};

struct FCommandParseResult {
  bool bMatched;
  FString CommandKey;
  TArray<FString> Args;
};

inline const TArray<FCommandRoute> &NodeParityRoutes() {
  static const TArray<FCommandRoute> Routes = {
      // BEGIN_NODE_CLI_COMMAND_KEYS
      {TEXT("version"), TEXT("version"), TEXT("")},
      {TEXT("doctor"), TEXT("doctor"), TEXT("")},
      {TEXT("status"), TEXT("status"), TEXT("")},
      {TEXT("setup"), TEXT("setup"), TEXT("")},
      {TEXT("setup_check"), TEXT("setup"), TEXT("check")},
      {TEXT("config_set"), TEXT("config"), TEXT("set")},
      {TEXT("config_get"), TEXT("config"), TEXT("get")},
      {TEXT("config_list"), TEXT("config"), TEXT("list")},
      {TEXT("npc_create"), TEXT("npc"), TEXT("create")},
      {TEXT("npc_state"), TEXT("npc"), TEXT("state")},
      {TEXT("npc_update"), TEXT("npc"), TEXT("update")},
      {TEXT("npc_process"), TEXT("npc"), TEXT("process")},
      {TEXT("npc_chat"), TEXT("npc"), TEXT("chat")},
      {TEXT("npc_import"), TEXT("npc"), TEXT("import")},
      {TEXT("soul_export"), TEXT("soul"), TEXT("export")},
      {TEXT("soul_import"), TEXT("soul"), TEXT("import")},
      {TEXT("soul_list"), TEXT("soul"), TEXT("list")},
      {TEXT("soul_chat"), TEXT("soul"), TEXT("chat")},
      {TEXT("soul_verify"), TEXT("soul"), TEXT("verify")},
      {TEXT("rules_list"), TEXT("rules"), TEXT("list")},
      {TEXT("rules_presets"), TEXT("rules"), TEXT("presets")},
      {TEXT("rules_register"), TEXT("rules"), TEXT("register")},
      {TEXT("rules_delete"), TEXT("rules"), TEXT("delete")},
      {TEXT("ghost_run"), TEXT("ghost"), TEXT("run")},
      {TEXT("ghost_status"), TEXT("ghost"), TEXT("status")},
      {TEXT("ghost_results"), TEXT("ghost"), TEXT("results")},
      {TEXT("ghost_stop"), TEXT("ghost"), TEXT("stop")},
      {TEXT("ghost_history"), TEXT("ghost"), TEXT("history")},
      {TEXT("memory_list"), TEXT("memory"), TEXT("list")},
      {TEXT("memory_recall"), TEXT("memory"), TEXT("recall")},
      {TEXT("memory_store"), TEXT("memory"), TEXT("store")},
      {TEXT("memory_clear"), TEXT("memory"), TEXT("clear")},
      {TEXT("memory_export"), TEXT("memory"), TEXT("export")},
      {TEXT("bridge_validate"), TEXT("bridge"), TEXT("validate")},
      {TEXT("bridge_rules"), TEXT("bridge"), TEXT("rules")},
      {TEXT("bridge_preset"), TEXT("bridge"), TEXT("preset")},
      // END_NODE_CLI_COMMAND_KEYS
  };
  return Routes;
}

inline const TArray<FString> &UeOnlyCommandKeys() {
  static const TArray<FString> Keys = {
      TEXT("npc_list"),
      TEXT("npc_active"),
      TEXT("soul_import_npc"),
      TEXT("system_status"),
      TEXT("vector_init"),
      TEXT("setup_deps"),
      TEXT("setup_verify"),
      TEXT("setup_runtime_check"),
      TEXT("test_game"),
  };
  return Keys;
}

inline bool ContainsRouteKeyRecursive(const TArray<FCommandRoute> &Routes,
                                      const FString &Key, int32 Index) {
  return Index >= Routes.Num()
             ? false
             : Routes[Index].Key == Key
                   ? true
                   : ContainsRouteKeyRecursive(Routes, Key, Index + 1);
}

inline bool ContainsStringRecursive(const TArray<FString> &Values,
                                    const FString &Value, int32 Index) {
  return Index >= Values.Num()
             ? false
             : Values[Index] == Value
                   ? true
                   : ContainsStringRecursive(Values, Value, Index + 1);
}

inline bool IsNodeParityCommandKey(const FString &Key) {
  return ContainsRouteKeyRecursive(NodeParityRoutes(), Key, 0);
}

inline bool IsUeOnlyCommandKey(const FString &Key) {
  return ContainsStringRecursive(UeOnlyCommandKeys(), Key, 0);
}

inline bool IsValidCommandKey(const FString &Key) {
  return IsNodeParityCommandKey(Key) || IsUeOnlyCommandKey(Key);
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
