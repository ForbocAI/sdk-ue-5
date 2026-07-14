#include "Features/CLI/Command/CommandAdapters.h"
#include "Core/fp.hpp"
#include "Misc/Parse.h"

namespace {

FString ExtractParam(const FString &Params, const TCHAR *Name) {
  FString Value;
  FParse::Value(*Params, Name, Value);
  return Value;
}

void AddIfPresent(TArray<FString> &Args, const FString &Value) {
  !Value.IsEmpty() ? (void)Args.Add(Value) : (void)0;
}

TArray<FString> BuildParamsRecursive(const FString &Params,
                                     const TCHAR *const *Names, int32 Count,
                                     int32 Index, TArray<FString> Args) {
  return Index >= Count
             ? Args
             : (AddIfPresent(Args, ExtractParam(Params, Names[Index])),
                BuildParamsRecursive(Params, Names, Count, Index + 1,
                                     MoveTemp(Args)));
}

TArray<FString> BuildParams(const FString &Params,
                            std::initializer_list<const TCHAR *> Names) {
  return BuildParamsRecursive(Params, Names.begin(),
                              static_cast<int32>(Names.size()), 0,
                              TArray<FString>());
}

TArray<FString> BuildFlagsRecursive(const FString &Params,
                                    const TCHAR *const *Names,
                                    const TCHAR *const *Values, int32 Count,
                                    int32 Index, TArray<FString> Args) {
  return Index >= Count
             ? Args
             : (FParse::Param(*Params, Names[Index])
                    ? (void)Args.Add(Values[Index])
                    : (void)0,
                BuildFlagsRecursive(Params, Names, Values, Count, Index + 1,
                                    MoveTemp(Args)));
}

TArray<FString> BuildFlags(const FString &Params,
                           std::initializer_list<const TCHAR *> Names,
                           std::initializer_list<const TCHAR *> Values) {
  return BuildFlagsRecursive(Params, Names.begin(), Values.begin(),
                             static_cast<int32>(Names.size()), 0,
                             TArray<FString>());
}

} // namespace

namespace CommandAdapters {

TArray<FString> BuildCommandArgs(const FString &Command,
                                 const FString &Params) {
  return func::or_else(
      func::multi_match<FString, TArray<FString>>(
          Command,
          {
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_create")),
                  [&Params](const FString &) {
                    TArray<FString> Args;
                    Args.Add(ExtractParam(Params, TEXT("Persona=")));
                    return Args;
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_process")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Id="), TEXT("Input=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_update")),
                  [&Params](const FString &) {
                    return BuildParams(
                        Params, {TEXT("Id="), TEXT("Mood="),
                                 TEXT("Inventory=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_import")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("TxId=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_chat")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Id="), TEXT("Message=")});
                  }),
              func::when<FString, TArray<FString>>(
                  [](const FString &Value) {
                    return Value == TEXT("memory_list") ||
                           Value == TEXT("memory_clear") ||
                           Value == TEXT("memory_export");
                  },
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Id=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("memory_recall")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Id="), TEXT("Query=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("memory_store")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Id="), TEXT("Obs=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("ghost_run")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Suite="), TEXT("Duration=")});
                  }),
              func::when<FString, TArray<FString>>(
                  [](const FString &Value) {
                    return Value == TEXT("ghost_status") ||
                           Value == TEXT("ghost_results") ||
                           Value == TEXT("ghost_stop");
                  },
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("SessionId=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("ghost_history")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Limit=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("bridge_validate")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Action=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("bridge_preset")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Name=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("rules_register")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Json=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("rules_delete")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Id=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("soul_export")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Id=")});
                  }),
              func::when<FString, TArray<FString>>(
                  [](const FString &Value) {
                    return Value == TEXT("soul_import") ||
                           Value == TEXT("soul_verify");
                  },
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("TxId=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("soul_chat")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Id="), TEXT("Message=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("soul_list")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Limit=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("config_set")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                       {TEXT("Key="), TEXT("Value=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("config_get")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Key=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("setup")),
                  [&Params](const FString &) {
                    return BuildFlags(Params, {TEXT("SqliteOnly")},
                                      {TEXT("--sqlite-only")});
                  }),
          }),
      TArray<FString>());
}

} // namespace CommandAdapters
