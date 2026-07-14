#include "Features/CLI/Invocation/InvocationAdapters.h"
#include "Features/CLI/CLIAdapters.h"
#include "Features/CLI/Command/CommandAdapters.h"
#include "Misc/Parse.h"

namespace {

struct FNormalizedTokens {
  TArray<FString> Tokens;
  FString ApiUrl;
  FString ApiKey;
};

TArray<FString> BuildCliArgTokensRecursive(const FString &Params, int32 Index,
                                           TArray<FString> Tokens) {
  FString Value;
  const FString ParamName = FString::Printf(TEXT("CliArg%d="), Index);
  return !FParse::Value(*Params, *ParamName, Value)
             ? Tokens
             : (Tokens.Add(Value),
                BuildCliArgTokensRecursive(Params, Index + 1,
                                           MoveTemp(Tokens)));
}

FNormalizedTokens NormalizeTokensRecursive(const TArray<FString> &RawTokens,
                                            int32 Index,
                                            FNormalizedTokens Normalized) {
  const FString Token =
      Index < RawTokens.Num() ? RawTokens[Index] : FString(TEXT(""));
  return Index >= RawTokens.Num()
             ? Normalized
             : ((Token == TEXT("--api-url") ||
                 Token == TEXT("--apiUrl")) &&
                Index + 1 < RawTokens.Num())
                   ? (Normalized.ApiUrl = RawTokens[Index + 1],
                      NormalizeTokensRecursive(RawTokens, Index + 2,
                                               MoveTemp(Normalized)))
                   : Token.StartsWith(TEXT("--api-url="))
                         ? (Normalized.ApiUrl = Token.Mid(
                                FString(TEXT("--api-url=")).Len()),
                            NormalizeTokensRecursive(RawTokens, Index + 1,
                                                     MoveTemp(Normalized)))
                         : Token.StartsWith(TEXT("--apiUrl="))
                               ? (Normalized.ApiUrl = Token.Mid(
                                      FString(TEXT("--apiUrl=")).Len()),
                                  NormalizeTokensRecursive(
                                      RawTokens, Index + 1,
                                      MoveTemp(Normalized)))
                               : ((Token == TEXT("--api-key") ||
                                   Token == TEXT("--apiKey")) &&
                                  Index + 1 < RawTokens.Num())
                                     ? (Normalized.ApiKey =
                                            RawTokens[Index + 1],
                                        NormalizeTokensRecursive(
                                            RawTokens, Index + 2,
                                            MoveTemp(Normalized)))
                                     : Token.StartsWith(TEXT("--api-key="))
                                           ? (Normalized.ApiKey = Token.Mid(
                                                  FString(TEXT("--api-key="))
                                                      .Len()),
                                              NormalizeTokensRecursive(
                                                  RawTokens, Index + 1,
                                                  MoveTemp(Normalized)))
                                           : Token.StartsWith(
                                                 TEXT("--apiKey="))
                                                 ? (Normalized.ApiKey =
                                                        Token.Mid(FString(
                                                            TEXT("--apiKey="))
                                                                      .Len()),
                                                    NormalizeTokensRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Normalized)))
                                                 : (Normalized.Tokens.Add(
                                                        Token),
                                                    NormalizeTokensRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Normalized)));
}

FString ExtractValue(const FString &Params, const TCHAR *Name) {
  FString Value;
  FParse::Value(*Params, Name, Value);
  return Value;
}

} // namespace

namespace CommandletInvocation {

FInvocation ResolveInvocation(const FString &Params) {
  const FString DirectCommand = ExtractValue(Params, TEXT("Command="));
  const FNormalizedTokens Normalized = NormalizeTokensRecursive(
      BuildCliArgTokensRecursive(Params, 0, TArray<FString>()), 0,
      FNormalizedTokens());
  const ForbocAI::CLI::FCommandParseResult Parsed =
      ForbocAI::CLI::ParseCommandTokens(Normalized.Tokens);
  const bool bUseCliTokens = DirectCommand.IsEmpty() && Parsed.bMatched;

  FInvocation Invocation;
  Invocation.Command =
      !DirectCommand.IsEmpty()
          ? DirectCommand
          : bUseCliTokens
                ? Parsed.CommandKey
                : (Normalized.Tokens.Num() > 0
                       ? Normalized.Tokens[0]
                       : FString(TEXT("")));
  Invocation.Args = bUseCliTokens
                        ? Parsed.Args
                        : CommandAdapters::BuildCommandArgs(Invocation.Command,
                                                            Params);

  const FString DirectApiUrl = ExtractValue(Params, TEXT("ApiUrl="));
  const FString DirectApiKey = ExtractValue(Params, TEXT("ApiKey="));
  Invocation.ApiUrl =
      !DirectApiUrl.IsEmpty() ? DirectApiUrl : Normalized.ApiUrl;
  Invocation.ApiKey =
      !DirectApiKey.IsEmpty() ? DirectApiKey : Normalized.ApiKey;
  return Invocation;
}

} // namespace CommandletInvocation
