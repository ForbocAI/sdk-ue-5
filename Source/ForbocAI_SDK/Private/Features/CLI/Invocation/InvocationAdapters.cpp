#include "Features/CLI/Invocation/InvocationAdapters.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/Data/DataAdapters.h"
#include "Misc/Parse.h"

namespace {

using CommandletInvocation::FInvocationTransportSettings;

struct FNormalizedTokens {
  TArray<FString> Tokens;
  FString ApiUrl;
  FString ApiKey;
};

struct FOptionMatch {
  FString Value;
  int32 NextIndex;
};

/** User Story: As a features cli invocation consumer, I need to invoke read invocation transport settings through a stable signature so the features cli invocation workflow remains explicit and composable. @fn FInvocationTransportSettings ReadInvocationTransportSettings() */
FInvocationTransportSettings ReadInvocationTransportSettings() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/cli/invocation.json"));
  const TSharedRef<FJsonObject> Commandlet =
      DataAdapters::ReadObjectField(Source, TEXT("commandlet"));
  FInvocationTransportSettings Settings;
  Settings.ArgumentParameterTemplate = DataAdapters::ReadStringField(
      Commandlet, TEXT("argumentParameterTemplate"));
  Settings.IndexToken =
      DataAdapters::ReadStringField(Commandlet, TEXT("indexToken"));
  Settings.FirstArgumentIndex =
      DataAdapters::ReadNumberField(Commandlet, TEXT("firstArgumentIndex"));
  Settings.NextArgumentOffset =
      DataAdapters::ReadNumberField(Commandlet, TEXT("nextArgumentOffset"));
  return Settings;
}

/** User Story: As a features cli invocation consumer, I need to invoke invocation transport settings through a stable signature so the features cli invocation workflow remains explicit and composable. @fn const FInvocationTransportSettings &InvocationTransportSettings() */
const FInvocationTransportSettings &InvocationTransportSettings() {
  static const func::Lazy<FInvocationTransportSettings> Settings =
      func::lazy([]() { return ReadInvocationTransportSettings(); });
  return func::eval(Settings);
}

/** User Story: As a features cli invocation consumer, I need to invoke build cli arg tokens recursive through a stable signature so the features cli invocation workflow remains explicit and composable. @fn TArray<FString> BuildCliArgTokensRecursive( const FString &Params, const FInvocationTransportSettings &Settings, int32 Index, TArray<FString> Tokens) */
TArray<FString> BuildCliArgTokensRecursive(
    const FString &Params, const FInvocationTransportSettings &Settings,
    int32 Index, TArray<FString> Tokens) {
  FString Value;
  const FString ParamName = Settings.ArgumentParameterTemplate.Replace(
      *Settings.IndexToken, *FString::FromInt(Index));
  return !FParse::Value(*Params, *ParamName, Value)
             ? Tokens
             : (Tokens.Add(Value),
                BuildCliArgTokensRecursive(
                    Params, Settings,
                    Index + Settings.NextArgumentOffset,
                    MoveTemp(Tokens)));
}

/** User Story: As a features cli invocation consumer, I need to invoke match assigned option recursive through a stable signature so the features cli invocation workflow remains explicit and composable. @fn func::Maybe<FOptionMatch> MatchAssignedOptionRecursive( const FString &Token, const TArray<FString> &Names, const ForbocAI::CLI::FCLIInvocationSettings &Options, const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 Index, int32 NextTokenIndex) */
func::Maybe<FOptionMatch> MatchAssignedOptionRecursive(
    const FString &Token, const TArray<FString> &Names,
    const ForbocAI::CLI::FCLIInvocationSettings &Options,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 Index,
    int32 NextTokenIndex) {
  return Index >= Names.Num()
             ? func::nothing<FOptionMatch>()
             : [&]() -> func::Maybe<FOptionMatch> {
                 const FString Prefix =
                     Names[Index] + Options.AssignmentSeparator;
                 return Token.StartsWith(Prefix)
                            ? func::just<FOptionMatch>(
                                  {Token.Mid(Prefix.Len()), NextTokenIndex})
                            : MatchAssignedOptionRecursive(
                                  Token, Names, Options, Parsing,
                                  Index + Parsing.NextIndexOffset,
                                  NextTokenIndex);
               }();
}

/** User Story: As a features cli invocation consumer, I need to invoke match separate option recursive through a stable signature so the features cli invocation workflow remains explicit and composable. @fn func::Maybe<FOptionMatch> MatchSeparateOptionRecursive( const TArray<FString> &RawTokens, const TArray<FString> &Names, const ForbocAI::CLI::FCLIInvocationSettings &Options, const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 TokenIndex, int32 NameIndex) */
func::Maybe<FOptionMatch> MatchSeparateOptionRecursive(
    const TArray<FString> &RawTokens, const TArray<FString> &Names,
    const ForbocAI::CLI::FCLIInvocationSettings &Options,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 TokenIndex,
    int32 NameIndex) {
  const int32 ValueIndex = TokenIndex + Options.OptionValueOffset;
  return NameIndex >= Names.Num() || ValueIndex >= RawTokens.Num()
             ? func::nothing<FOptionMatch>()
             : RawTokens[TokenIndex] == Names[NameIndex]
                   ? func::just<FOptionMatch>(
                          {RawTokens[ValueIndex],
                          ValueIndex + Parsing.NextIndexOffset})
                   : MatchSeparateOptionRecursive(
                         RawTokens, Names, Options, Parsing, TokenIndex,
                         NameIndex + Parsing.NextIndexOffset);
}

/** User Story: As a features cli invocation consumer, I need to invoke match option through a stable signature so the features cli invocation workflow remains explicit and composable. @fn func::Maybe<FOptionMatch> MatchOption( const TArray<FString> &RawTokens, const TArray<FString> &Names, const ForbocAI::CLI::FCLIInvocationSettings &Options, const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 TokenIndex) */
func::Maybe<FOptionMatch> MatchOption(
    const TArray<FString> &RawTokens, const TArray<FString> &Names,
    const ForbocAI::CLI::FCLIInvocationSettings &Options,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    int32 TokenIndex) {
  const int32 NextTokenIndex =
      TokenIndex + Parsing.NextIndexOffset;
  const func::Maybe<FOptionMatch> Assigned =
      MatchAssignedOptionRecursive(
          RawTokens[TokenIndex], Names, Options, Parsing,
          Parsing.FirstTokenIndex, NextTokenIndex);
  return Assigned.hasValue
             ? Assigned
             : MatchSeparateOptionRecursive(
                   RawTokens, Names, Options, Parsing, TokenIndex,
                   Parsing.FirstTokenIndex);
}

/** User Story: As a features cli invocation consumer, I need to invoke normalize tokens recursive through a stable signature so the features cli invocation workflow remains explicit and composable. @fn FNormalizedTokens NormalizeTokensRecursive( const TArray<FString> &RawTokens, const ForbocAI::CLI::FCLIInvocationSettings &Options, const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 Index, FNormalizedTokens Normalized) */
FNormalizedTokens NormalizeTokensRecursive(
    const TArray<FString> &RawTokens,
    const ForbocAI::CLI::FCLIInvocationSettings &Options,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing, int32 Index,
    FNormalizedTokens Normalized) {
  const FString Token =
      Index < RawTokens.Num() ? RawTokens[Index] : FString();
  const func::Maybe<FOptionMatch> ApiUrl =
      Index < RawTokens.Num()
          ? MatchOption(RawTokens, Options.ApiUrlOptions, Options, Parsing,
                        Index)
          : func::nothing<FOptionMatch>();
  const func::Maybe<FOptionMatch> ApiKey =
      Index < RawTokens.Num()
          ? MatchOption(RawTokens, Options.ApiKeyOptions, Options, Parsing,
                        Index)
          : func::nothing<FOptionMatch>();
  return Index >= RawTokens.Num()
             ? Normalized
             : ApiUrl.hasValue
                   ? (Normalized.ApiUrl = ApiUrl.value.Value,
                      NormalizeTokensRecursive(
                          RawTokens, Options, Parsing,
                          ApiUrl.value.NextIndex,
                          MoveTemp(Normalized)))
                   : ApiKey.hasValue
                         ? (Normalized.ApiKey = ApiKey.value.Value,
                            NormalizeTokensRecursive(
                                RawTokens, Options, Parsing,
                                ApiKey.value.NextIndex,
                                MoveTemp(Normalized)))
                         : (Normalized.Tokens.Add(Token),
                            NormalizeTokensRecursive(
                                RawTokens, Options, Parsing,
                                Index + Parsing.NextIndexOffset,
                                MoveTemp(Normalized)));
}

} // namespace

namespace CommandletInvocation {

/** User Story: As a features cli invocation consumer, I need to invoke resolve invocation through a stable signature so the features cli invocation workflow remains explicit and composable. @fn FInvocation ResolveInvocation( const FString &Params, const ForbocAI::CLI::FCLIState &CLIState) */
FInvocation ResolveInvocation(
    const FString &Params, const ForbocAI::CLI::FCLIState &CLIState) {
  const FInvocationTransportSettings &Transport =
      InvocationTransportSettings();
  const FNormalizedTokens Normalized = NormalizeTokensRecursive(
      BuildCliArgTokensRecursive(
          Params, Transport, Transport.FirstArgumentIndex,
          TArray<FString>()),
      CLIState.Invocation, CLIState.Parsing,
      CLIState.Parsing.FirstTokenIndex,
      FNormalizedTokens());
  const ForbocAI::CLI::FCommandParseResult Parsed =
      ForbocAI::CLI::resolveNodeCliCommand(CLIState, Normalized.Tokens);

  FInvocation Invocation;
  Invocation.Command =
      Parsed.bMatched
          ? Parsed.CommandKey
          : (Normalized.Tokens.Num() > CLIState.Parsing.FirstTokenIndex
                 ? Normalized.Tokens[CLIState.Parsing.FirstTokenIndex]
                 : FString());
  Invocation.Args = Parsed.bMatched ? Parsed.Args : TArray<FString>();
  Invocation.ApiUrl = Normalized.ApiUrl;
  Invocation.ApiKey = Normalized.ApiKey;
  return Invocation;
}

} // namespace CommandletInvocation
