#include "TestGame/CLI/TestGameCommandlet.h"

#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameThunks.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"
#include "TestGame/TestGameStore.h"
#include "TestGame/Views/Terminal/TerminalView.h"
#include "Misc/Parse.h"

namespace {

struct FTestGameCliInvocation {
  TArray<FString> Tokens;
  FString ApiUrl;
  FString ApiKey;
};

/** User Story: As a cli consumer, I need to invoke build cli arg tokens recursive through a stable signature so the cli workflow remains explicit and composable. @fn TArray<FString> BuildCliArgTokensRecursive(const FString &Params, int32 Index, TArray<FString> Tokens) */
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

/** User Story: As a cli consumer, I need to invoke build cli arg tokens through a stable signature so the cli workflow remains explicit and composable. @fn TArray<FString> BuildCliArgTokens(const FString &Params) */
TArray<FString> BuildCliArgTokens(const FString &Params) {
  return BuildCliArgTokensRecursive(Params, 0, TArray<FString>());
}

/** User Story: As a cli consumer, I need to invoke is api url flag through a stable signature so the cli workflow remains explicit and composable. @fn bool IsApiUrlFlag(const FString &Token) */
bool IsApiUrlFlag(const FString &Token) {
  return Token == TEXT("--api-url") || Token == TEXT("--apiUrl");
}

/** User Story: As a cli consumer, I need to invoke is api key flag through a stable signature so the cli workflow remains explicit and composable. @fn bool IsApiKeyFlag(const FString &Token) */
bool IsApiKeyFlag(const FString &Token) {
  return Token == TEXT("--api-key") || Token == TEXT("--apiKey");
}

/** User Story: As a cli consumer, I need to invoke normalize invocation recursive through a stable signature so the cli workflow remains explicit and composable. @fn FTestGameCliInvocation NormalizeInvocationRecursive( const TArray<FString> &RawTokens, int32 Index, FTestGameCliInvocation Invocation) */
FTestGameCliInvocation NormalizeInvocationRecursive(
    const TArray<FString> &RawTokens, int32 Index,
    FTestGameCliInvocation Invocation) {
  const FString Token =
      Index < RawTokens.Num() ? RawTokens[Index] : FString(TEXT(""));
  return Index >= RawTokens.Num()
             ? Invocation
             : (IsApiUrlFlag(Token) && Index + 1 < RawTokens.Num())
                   ? (Invocation.ApiUrl = RawTokens[Index + 1],
                      NormalizeInvocationRecursive(RawTokens, Index + 2,
                                                   MoveTemp(Invocation)))
                   : Token.StartsWith(TEXT("--api-url="))
                         ? (Invocation.ApiUrl =
                                Token.Mid(FString(TEXT("--api-url=")).Len()),
                            NormalizeInvocationRecursive(
                                RawTokens, Index + 1, MoveTemp(Invocation)))
                         : Token.StartsWith(TEXT("--apiUrl="))
                               ? (Invocation.ApiUrl = Token.Mid(
                                      FString(TEXT("--apiUrl=")).Len()),
                                  NormalizeInvocationRecursive(
                                      RawTokens, Index + 1,
                                      MoveTemp(Invocation)))
                               : (IsApiKeyFlag(Token) &&
                                  Index + 1 < RawTokens.Num())
                                     ? (Invocation.ApiKey =
                                            RawTokens[Index + 1],
                                        NormalizeInvocationRecursive(
                                            RawTokens, Index + 2,
                                            MoveTemp(Invocation)))
                                     : Token.StartsWith(TEXT("--api-key="))
                                           ? (Invocation.ApiKey = Token.Mid(
                                                  FString(TEXT("--api-key="))
                                                      .Len()),
                                              NormalizeInvocationRecursive(
                                                  RawTokens, Index + 1,
                                                  MoveTemp(Invocation)))
                                           : Token.StartsWith(TEXT("--apiKey="))
                                                 ? (Invocation.ApiKey =
                                                        Token.Mid(FString(
                                                                      TEXT("--apiKey="))
                                                                      .Len()),
                                                    NormalizeInvocationRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Invocation)))
                                                 : (Invocation.Tokens.Add(
                                                        Token),
                                                    NormalizeInvocationRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Invocation)));
}

/** User Story: As a cli consumer, I need to invoke normalize invocation through a stable signature so the cli workflow remains explicit and composable. @fn FTestGameCliInvocation NormalizeInvocation(const TArray<FString> &RawTokens) */
FTestGameCliInvocation NormalizeInvocation(const TArray<FString> &RawTokens) {
  return NormalizeInvocationRecursive(RawTokens, 0, FTestGameCliInvocation());
}

/** User Story: As a cli consumer, I need to invoke token after recursive through a stable signature so the cli workflow remains explicit and composable. @fn FString TokenAfterRecursive(const TArray<FString> &Tokens, const FString &Needle, int32 Index) */
FString TokenAfterRecursive(const TArray<FString> &Tokens,
                            const FString &Needle, int32 Index) {
  return Index >= Tokens.Num()
             ? FString(TEXT(""))
             : Tokens[Index] == Needle && Index + 1 < Tokens.Num()
                   ? Tokens[Index + 1]
                   : TokenAfterRecursive(Tokens, Needle, Index + 1);
}

/** User Story: As a cli consumer, I need to invoke has token recursive through a stable signature so the cli workflow remains explicit and composable. @fn bool HasTokenRecursive(const TArray<FString> &Tokens, const FString &Needle, int32 Index) */
bool HasTokenRecursive(const TArray<FString> &Tokens, const FString &Needle,
                       int32 Index) {
  return Index >= Tokens.Num()
             ? false
             : Tokens[Index] == Needle
                   ? true
                   : HasTokenRecursive(Tokens, Needle, Index + 1);
}

/** User Story: As a cli consumer, I need to invoke resolve mode through a stable signature so the cli workflow remains explicit and composable. @fn FString ResolveMode(const TArray<FString> &Tokens) */
FString ResolveMode(const TArray<FString> &Tokens) {
  const FString ExplicitMode = TokenAfterRecursive(Tokens, TEXT("--mode"), 0);
  return !ExplicitMode.IsEmpty()
             ? ExplicitMode
             : (Tokens.Num() > 0 &&
                (Tokens[0] == TEXT("autoplay") || Tokens[0] == TEXT("manual")))
                   ? Tokens[0]
                   : FString(TEXT("autoplay"));
}

/** User Story: As a cli consumer, I need to invoke resolve api url through a stable signature so the cli workflow remains explicit and composable. @fn FString ResolveApiUrl(const FTestGameCliInvocation &Invocation) */
FString ResolveApiUrl(const FTestGameCliInvocation &Invocation) {
  return !Invocation.ApiUrl.IsEmpty() ? Invocation.ApiUrl
                                      : SDKConfig::GetApiUrl();
}

/** User Story: As a cli consumer, I need to invoke resolve api key through a stable signature so the cli workflow remains explicit and composable. @fn FString ResolveApiKey(const FTestGameCliInvocation &Invocation) */
FString ResolveApiKey(const FTestGameCliInvocation &Invocation) {
  return !Invocation.ApiKey.IsEmpty() ? Invocation.ApiKey
                                      : SDKConfig::GetApiKey();
}

/** User Story: As a cli consumer, I need to invoke parse play mode through a stable signature so the cli workflow remains explicit and composable. @fn TestGame::EPlayMode ParsePlayMode(const FString &Mode) */
TestGame::EPlayMode ParsePlayMode(const FString &Mode) {
  return Mode == TEXT("manual") ? TestGame::EPlayMode::Manual
                                : TestGame::EPlayMode::Autoplay;
}

/** User Story: As a cli consumer, I need to invoke print usage through a stable signature so the cli workflow remains explicit and composable. @fn void PrintUsage() */
void PrintUsage() {
  TestGame::PresentProgress(TestGame::SelectUsageViewModel());
}

/** User Story: As a cli consumer, I need to invoke run contract command through a stable signature so the cli workflow remains explicit and composable. @fn int32 RunContractCommand(const FString &ApiUrl) */
int32 RunContractCommand(const FString &ApiUrl) {
  TestGame::FTestGameStore Store = TestGame::createTestGameStore();
  const TestGame::Contract::FRawContractResponse Raw =
      TestGame::Contract::GetContractJson(Store, ApiUrl);
  TestGame::PresentProgress(TestGame::SelectContractViewModel(Raw));
  return Raw.bSuccess ? 0 : 1;
}

/** User Story: As a cli consumer, I need to invoke run game command through a stable signature so the cli workflow remains explicit and composable. @fn int32 RunGameCommand(const FString &Mode, const FString &ApiUrl) */
int32 RunGameCommand(const FString &Mode, const FString &ApiUrl) {
  TestGame::FTestGameStore Store = TestGame::createTestGameStore();
  const TestGame::FGameProgressSink ProgressSink =
      [](const TestGame::FGameProgress &Progress) {
        TestGame::PresentProgress(
            TestGame::SelectTerminalProgressViewModel(Progress));
      };
  const TestGame::FGameRunResult Result =
      TestGame::RunGame(Store, ParsePlayMode(Mode), ApiUrl, ProgressSink);
  return Result.bComplete ? 0 : 2;
}

} // namespace

/** User Story: As a cli consumer, I need to invoke uforboc aitest game commandlet through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAITestGameCommandlet::UForbocAITestGameCommandlet() */
UForbocAITestGameCommandlet::UForbocAITestGameCommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/** User Story: As a cli consumer, I need to invoke main through a stable signature so the cli workflow remains explicit and composable. @fn int32 UForbocAITestGameCommandlet::Main(const FString &Params) */
int32 UForbocAITestGameCommandlet::Main(const FString &Params) {
  SDKConfig::InitializeConfig();
  const FTestGameCliInvocation Invocation =
      NormalizeInvocation(BuildCliArgTokens(Params));
  const FString ApiUrl = ResolveApiUrl(Invocation);
  SDKConfig::SetApiConfig(ApiUrl, ResolveApiKey(Invocation));

  return HasTokenRecursive(Invocation.Tokens, TEXT("--help"), 0) ||
                 HasTokenRecursive(Invocation.Tokens, TEXT("-h"), 0)
             ? (PrintUsage(), 0)
         : Invocation.Tokens.Num() > 0 &&
                   Invocation.Tokens[0] == TEXT("contract")
             ? RunContractCommand(ApiUrl)
             : [&]() {
                 const FString Mode = ResolveMode(Invocation.Tokens);
                 return Mode == TEXT("autoplay") || Mode == TEXT("manual")
                            ? RunGameCommand(Mode, ApiUrl)
                            : (TestGame::PresentProgress(
                                   TestGame::SelectInvalidModeViewModel(Mode)),
                               PrintUsage(), 2);
               }();
}
