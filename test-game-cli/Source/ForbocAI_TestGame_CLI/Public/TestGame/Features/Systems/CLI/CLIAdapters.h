#pragma once

#include "Features/Config/ConfigAdapters.h"
#include "Misc/Parse.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/CLI/CLITypes.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"

namespace TestGame::CLI {

/** User Story: As a test-game CLI maintainer, I need one immutable authored CLI contract so Windows, macOS, and Linux hosts parse the same commands and exit codes. @fn inline const FCliRuntimeData &cliRuntimeData() */
inline const FCliRuntimeData &cliRuntimeData() {
  static const FCliRuntimeData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("cli/runtime.json"));
    const TSharedRef<FJsonObject> Argv =
        DataAdapters::ReadObjectField(Source, TEXT("argv"));
    const TSharedRef<FJsonObject> Commands =
        DataAdapters::ReadObjectField(Source, TEXT("commands"));
    const TSharedRef<FJsonObject> ExitCodes =
        DataAdapters::ReadObjectField(Source, TEXT("exitCodes"));
    const TSharedRef<FJsonObject> Flags =
        DataAdapters::ReadObjectField(Source, TEXT("flags"));
    FCliRuntimeData Value;

#define FORBOCAI_READ_CLI_ARGV_FString(Name)                             \
  Value.argv.Name = DataAdapters::ReadStringField(Argv, TEXT(#Name));
#define FORBOCAI_READ_CLI_ARGV_int32(Name)                               \
  Value.argv.Name = DataAdapters::ReadNumberField(Argv, TEXT(#Name));
#define FORBOCAI_READ_CLI_ARGV(Type, Name)                               \
  FORBOCAI_READ_CLI_ARGV_##Type(Name)
    FORBOCAI_CLI_ARGV_FIELDS(FORBOCAI_READ_CLI_ARGV)
#undef FORBOCAI_READ_CLI_ARGV
#undef FORBOCAI_READ_CLI_ARGV_int32
#undef FORBOCAI_READ_CLI_ARGV_FString

#define FORBOCAI_READ_CLI_COMMAND(Type, Name)                            \
  Value.commands.Name = DataAdapters::ReadStringField(Commands, TEXT(#Name));
    FORBOCAI_CLI_COMMAND_FIELDS(FORBOCAI_READ_CLI_COMMAND)
#undef FORBOCAI_READ_CLI_COMMAND

#define FORBOCAI_READ_CLI_EXIT_CODE(Type, Name)                          \
  Value.exitCodes.Name = DataAdapters::ReadNumberField(ExitCodes, TEXT(#Name));
    FORBOCAI_CLI_EXIT_CODE_FIELDS(FORBOCAI_READ_CLI_EXIT_CODE)
#undef FORBOCAI_READ_CLI_EXIT_CODE

    Value.flags.help =
        DataAdapters::ReadStringArrayField(Flags, TEXT("help"));
    Value.flags.mode =
        DataAdapters::ReadStringField(Flags, TEXT("mode"));
    Value.flags.apiUrl =
        DataAdapters::ReadStringArrayField(Flags, TEXT("apiUrl"));
    Value.flags.apiKey =
        DataAdapters::ReadStringArrayField(Flags, TEXT("apiKey"));
    Value.flags.assignmentSeparator = DataAdapters::ReadStringField(
        Flags, TEXT("assignmentSeparator"));
    return Value;
  }();
  return Data;
}

namespace detail {

/** User Story: As a test-game CLI operator, I need commandlet argument slots decoded recursively so wrapper scripts can pass an arbitrary command without platform-specific limits. @fn inline TArray<FString> buildCliArgTokensRecursive( const FString &Params, int32 Index, TArray<FString> Tokens) */
inline TArray<FString> buildCliArgTokensRecursive(
    const FString &Params, int32 Index, TArray<FString> Tokens) {
  FString Value;
  const FCliRuntimeData &Data = cliRuntimeData();
  const FString ParamName =
      FString::Format(*Data.argv.cliArgTemplate, {Index});
  return !FParse::Value(*Params, *ParamName, Value)
             ? Tokens
             : (Tokens.Add(Value),
                buildCliArgTokensRecursive(
                    Params, Index + Data.argv.nextIndex, MoveTemp(Tokens)));
}

/** User Story: As a test-game CLI operator, I need token membership checked recursively so aliases stay authored and parsing remains deterministic. @fn inline bool hasToken(const TArray<FString> &Tokens, const FString &Needle, int32 Index) */
inline bool hasToken(const TArray<FString> &Tokens, const FString &Needle,
                     int32 Index) {
  return Index >= Tokens.Num()
             ? false
             : Tokens[Index] == Needle
                   ? true
                   : hasToken(Tokens, Needle,
                              Index + cliRuntimeData().argv.nextIndex);
}

/** User Story: As a test-game CLI operator, I need authored aliases matched recursively so all supported flag spellings share one parser path. @fn inline bool hasAnyToken(const TArray<FString> &Tokens, const TArray<FString> &Needles, int32 Index) */
inline bool hasAnyToken(const TArray<FString> &Tokens,
                        const TArray<FString> &Needles, int32 Index) {
  return Index >= Needles.Num()
             ? false
             : hasToken(Tokens, Needles[Index],
                        cliRuntimeData().argv.firstIndex)
                   ? true
                   : hasAnyToken(Tokens, Needles,
                                 Index + cliRuntimeData().argv.nextIndex);
}

/** User Story: As a test-game CLI operator, I need assignment-form aliases recognized recursively so transport options are removed without imperative loops. @fn inline bool startsWithAny(const FString &Token, const TArray<FString> &Aliases, int32 Index) */
inline bool startsWithAny(const FString &Token,
                          const TArray<FString> &Aliases, int32 Index) {
  return Index >= Aliases.Num()
             ? false
             : Token.StartsWith(Aliases[Index] +
                                cliRuntimeData().flags.assignmentSeparator)
                   ? true
                   : startsWithAny(
                         Token, Aliases,
                         Index + cliRuntimeData().argv.nextIndex);
}

/** User Story: As a test-game CLI operator, I need both split and assignment flag forms decoded through one adapter so shell choice never changes API configuration. @fn inline FString readFlagValue(const TArray<FString> &Tokens, const TArray<FString> &Aliases, int32 TokenIndex, int32 AliasIndex) */
inline FString readFlagValue(const TArray<FString> &Tokens,
                             const TArray<FString> &Aliases,
                             int32 TokenIndex, int32 AliasIndex) {
  const FCliRuntimeData &Data = cliRuntimeData();
  return TokenIndex >= Tokens.Num()
             ? FString()
         : AliasIndex >= Aliases.Num()
             ? readFlagValue(Tokens, Aliases,
                             TokenIndex + Data.argv.nextIndex,
                             Data.argv.firstIndex)
         : Tokens[TokenIndex] == Aliases[AliasIndex] &&
                   TokenIndex + Data.argv.valueOffset < Tokens.Num()
             ? Tokens[TokenIndex + Data.argv.valueOffset]
             : [&]() {
                 const FString Prefix =
                     Aliases[AliasIndex] + Data.flags.assignmentSeparator;
                 return Tokens[TokenIndex].StartsWith(Prefix)
                            ? Tokens[TokenIndex].Mid(Prefix.Len())
                            : readFlagValue(
                                  Tokens, Aliases, TokenIndex,
                                  AliasIndex + Data.argv.nextIndex);
               }();
}

/** User Story: As a test-game CLI operator, I need transport flags removed recursively so command and mode resolution operate only on semantic CLI tokens. @fn inline TArray<FString> removeFlagsRecursive( const TArray<FString> &Tokens, const TArray<FString> &Aliases, int32 Index, TArray<FString> Acc) */
inline TArray<FString> removeFlagsRecursive(
    const TArray<FString> &Tokens, const TArray<FString> &Aliases,
    int32 Index, TArray<FString> Acc) {
  const FCliRuntimeData &Data = cliRuntimeData();
  const bool bSplitFlag = Index < Tokens.Num() &&
                          hasAnyToken({Tokens[Index]}, Aliases,
                                      Data.argv.firstIndex);
  const bool bAssignedFlag =
      Index < Tokens.Num() &&
      startsWithAny(Tokens[Index], Aliases, Data.argv.firstIndex);
  return Index >= Tokens.Num()
             ? Acc
         : bSplitFlag
             ? removeFlagsRecursive(Tokens, Aliases,
                                    Index + Data.argv.pairOffset,
                                    MoveTemp(Acc))
         : bAssignedFlag
             ? removeFlagsRecursive(Tokens, Aliases,
                                    Index + Data.argv.nextIndex,
                                    MoveTemp(Acc))
             : (Acc.Add(Tokens[Index]),
                removeFlagsRecursive(Tokens, Aliases,
                                     Index + Data.argv.nextIndex,
                                     MoveTemp(Acc)));
}

} // namespace detail

/** User Story: As a test-game CLI host, I need commandlet parameters converted to semantic tokens through one adapter so host wiring contains no parsing policy. @fn inline TArray<FString> buildCliArgTokens(const FString &Params) */
inline TArray<FString> buildCliArgTokens(const FString &Params) {
  return detail::buildCliArgTokensRecursive(
      Params, cliRuntimeData().argv.firstIndex, TArray<FString>());
}

/** User Story: As a test-game CLI host, I need transport configuration separated from semantic arguments so API settings never leak into game-mode dispatch. @fn inline FCliInvocation normalizeInvocation(const TArray<FString> &RawTokens) */
inline FCliInvocation normalizeInvocation(const TArray<FString> &RawTokens) {
  const FCliRuntimeData &Data = cliRuntimeData();
  FCliInvocation Invocation;
  Invocation.ApiUrl = detail::readFlagValue(
      RawTokens, Data.flags.apiUrl, Data.argv.firstIndex,
      Data.argv.firstIndex);
  Invocation.ApiKey = detail::readFlagValue(
      RawTokens, Data.flags.apiKey, Data.argv.firstIndex,
      Data.argv.firstIndex);
  Invocation.Tokens = detail::removeFlagsRecursive(
      detail::removeFlagsRecursive(RawTokens, Data.flags.apiUrl,
                                   Data.argv.firstIndex, {}),
      Data.flags.apiKey, Data.argv.firstIndex, {});
  return Invocation;
}

/** User Story: As a test-game CLI host, I need help aliases evaluated from authored data so usage behavior is portable and testable. @fn inline bool requestsHelp(const FCliInvocation &Invocation) */
inline bool requestsHelp(const FCliInvocation &Invocation) {
  return detail::hasAnyToken(Invocation.Tokens, cliRuntimeData().flags.help,
                             cliRuntimeData().argv.firstIndex);
}

/** User Story: As a test-game CLI host, I need the authored command position selected safely so empty invocations never rely on sentinel literals. @fn inline FString selectCommand(const FCliInvocation &Invocation) */
inline FString selectCommand(const FCliInvocation &Invocation) {
  const int32 Index = cliRuntimeData().argv.commandIndex;
  return Invocation.Tokens.IsValidIndex(Index) ? Invocation.Tokens[Index]
                                                : FString();
}

/** User Story: As a test-game CLI host, I need mode selection derived from authored flags and runtime domains so invalid values cannot enter the root store. @fn inline FString resolveMode(const FCliInvocation &Invocation) */
inline FString resolveMode(const FCliInvocation &Invocation) {
  const FCliRuntimeData &Data = cliRuntimeData();
  const FString ExplicitMode = detail::readFlagValue(
      Invocation.Tokens, {Data.flags.mode}, Data.argv.firstIndex,
      Data.argv.firstIndex);
  const FString Command = selectCommand(Invocation);
  return !ExplicitMode.IsEmpty()
             ? ExplicitMode
             : GameAdapters::GameRuntimeData().modes.all.Contains(Command)
                   ? Command
                   : GameAdapters::GameRuntimeData().modes.autoplay;
}

/** User Story: As a test-game CLI host, I need API URL precedence resolved once so explicit invocations override the SDK environment consistently. @fn inline FString resolveApiUrl(const FCliInvocation &Invocation) */
inline FString resolveApiUrl(const FCliInvocation &Invocation) {
  return !Invocation.ApiUrl.IsEmpty() ? Invocation.ApiUrl
                                      : SDKConfig::GetApiUrl();
}

/** User Story: As a test-game CLI host, I need API key precedence resolved once so credentials flow through configuration without extra shell steps. @fn inline FString resolveApiKey(const FCliInvocation &Invocation) */
inline FString resolveApiKey(const FCliInvocation &Invocation) {
  return !Invocation.ApiKey.IsEmpty() ? Invocation.ApiKey
                                      : SDKConfig::GetApiKey();
}

} // namespace TestGame::CLI
