#include "RuntimeCommandlet.h"
#include "CLI/CliCommandMatrix.h"
#include "CLI/CLIModule.h"
#include "Core/ue_fp.hpp"
#include "Misc/Parse.h"
#include "RuntimeConfig.h"

namespace {

/**
 * Extracts a named param from UE command-line params string.
 * Returns empty string if not found.
 * User Story: As commandlet parsing, I need named parameter extraction so raw
 * Unreal params can be translated into structured command arguments.
 */
FString ExtractParam(const FString &Params, const TCHAR *ParamName) {
  FString Value;
  FParse::Value(*Params, ParamName, Value);
  return Value;
}

/**
 * Appends a param value to the args array if non-empty.
 * User Story: As commandlet parsing, I need optional args appended only when
 * present so handlers receive a clean positional argument list.
 */
void AddIfPresent(TArray<FString> &Args, const FString &Value) {
  !Value.IsEmpty() ? (void)Args.Add(Value) : (void)0;
}

/**
 * Appends a flag argument when the raw params include the named switch.
 * User Story: As commandlet parsing, I need boolean switches preserved so CLI
 * handlers can receive the same flag-style arguments as the direct CLI path.
 */
void AddFlagIfPresent(TArray<FString> &Args, const FString &Params,
                      const TCHAR *ParamName, const TCHAR *FlagValue) {
  FParse::Param(*Params, ParamName) ? (void)Args.Add(FlagValue) : (void)0;
}

/**
 * Extracts a named param and prepends a flag prefix when the value is present.
 * Returns empty string when the param is absent, so AddIfPresent will skip it.
 * User Story: As commandlet parsing, I need prefixed flag construction so raw
 * params translate into the --key=value format expected by CLI handlers.
 */
FString ExtractPrefixed(const FString &Params, const TCHAR *ParamName,
                        const TCHAR *Prefix) {
  const FString Raw = ExtractParam(Params, ParamName);
  return Raw.IsEmpty() ? FString(TEXT("")) : FString(Prefix) + Raw;
}

/**
 * Recursively appends extracted params to an args array.
 * User Story: As commandlet parsing, I need recursive param extraction so
 * multiple named params can be gathered without imperative loops.
 */
namespace commandlet_detail {
TArray<FString> buildParamsRecursive(const FString &Params,
                                     const TCHAR *const *Names, int32 Count,
                                     int32 Index, TArray<FString> Args) {
  return Index >= Count
             ? Args
             : (AddIfPresent(Args, ExtractParam(Params, Names[Index])),
                buildParamsRecursive(Params, Names, Count, Index + 1,
                                    MoveTemp(Args)));
}

/**
 * Recursively appends flag arguments when named switches are present.
 * Takes parallel arrays of param names and flag values.
 * User Story: As commandlet parsing, I need recursive flag extraction so
 * multiple boolean switches can be gathered without imperative loops.
 */
TArray<FString> buildFlagsRecursive(const FString &Params,
                                    const TCHAR *const *ParamNames,
                                    const TCHAR *const *FlagValues,
                                    int32 Count, int32 Index,
                                    TArray<FString> Args) {
  return Index >= Count
             ? Args
             : (AddFlagIfPresent(Args, Params, ParamNames[Index],
                                 FlagValues[Index]),
                buildFlagsRecursive(Params, ParamNames, FlagValues, Count,
                                   Index + 1, MoveTemp(Args)));
}

/**
 * Recursively appends prefixed params (--key=value) to an args array.
 * Takes parallel arrays of param names and prefixes.
 * User Story: As commandlet parsing, I need recursive prefixed extraction so
 * multiple --key=value flags can be gathered without imperative loops.
 */
TArray<FString> buildPrefixedRecursive(const FString &Params,
                                       const TCHAR *const *ParamNames,
                                       const TCHAR *const *Prefixes,
                                       int32 Count, int32 Index,
                                       TArray<FString> Args) {
  return Index >= Count
             ? Args
             : (AddIfPresent(Args,
                             ExtractPrefixed(Params, ParamNames[Index],
                                            Prefixes[Index])),
                buildPrefixedRecursive(Params, ParamNames, Prefixes, Count,
                                      Index + 1, MoveTemp(Args)));
}

/**
 * Recursively appends elements from a source array into a destination array.
 * User Story: As commandlet parsing, I need recursive array merging so
 * separately built param and flag lists can be combined declaratively.
 */
TArray<FString> mergeArraysRecursive(TArray<FString> Dest,
                                     const TArray<FString> &Src,
                                     int32 Index) {
  return Index >= Src.Num()
             ? Dest
             : (Dest.Add(Src[Index]),
                mergeArraysRecursive(MoveTemp(Dest), Src, Index + 1));
}
} // namespace commandlet_detail

/**
 * Extracts multiple named params and collects them into an args array.
 * User Story: As commandlet parsing, I need batch param extraction so commands
 * with several named params can build their args list declaratively.
 */
TArray<FString> BuildParams(const FString &Params,
                            std::initializer_list<const TCHAR *> Names) {
  return commandlet_detail::buildParamsRecursive(
      Params, Names.begin(), static_cast<int32>(Names.size()), 0,
      TArray<FString>());
}

/**
 * Extracts multiple boolean switches and collects them as flag args.
 * User Story: As commandlet parsing, I need batch flag extraction so commands
 * with several switches can build their args list declaratively.
 */
TArray<FString> BuildFlags(const FString &Params,
                           std::initializer_list<const TCHAR *> ParamNames,
                           std::initializer_list<const TCHAR *> FlagValues) {
  return commandlet_detail::buildFlagsRecursive(
      Params, ParamNames.begin(), FlagValues.begin(),
      static_cast<int32>(ParamNames.size()), 0, TArray<FString>());
}

/**
 * Extracts multiple named params with flag prefixes and collects them.
 * User Story: As commandlet parsing, I need batch prefixed extraction so
 * commands with --key=value flags can build their args list declaratively.
 */
TArray<FString> BuildPrefixed(const FString &Params,
                              std::initializer_list<const TCHAR *> ParamNames,
                              std::initializer_list<const TCHAR *> Prefixes) {
  return commandlet_detail::buildPrefixedRecursive(
      Params, ParamNames.begin(), Prefixes.begin(),
      static_cast<int32>(ParamNames.size()), 0, TArray<FString>());
}

/**
 * Merges two args arrays into one via recursive append.
 * User Story: As commandlet parsing, I need array merging so separately built
 * param and flag lists can be combined into one args array declaratively.
 */
TArray<FString> MergeArgs(TArray<FString> Base, const TArray<FString> &Extra) {
  return commandlet_detail::mergeArraysRecursive(MoveTemp(Base), Extra, 0);
}

struct FCliInvocation {
  TArray<FString> Tokens;
  FString ApiUrl;
  FString ApiKey;
};

/**
 * Extracts wrapper-provided TS-style CLI tokens from -CliArgN params.
 * User Story: As UE CLI users, I need Node-style argv routed through the
 * commandlet so the SDK checkout can expose the same command ergonomics as TS.
 */
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

/**
 * Pulls global CLI flags out of TS-style argv before command-key parsing.
 * User Story: As CLI parity, I need --api-url/--api-key accepted the same way
 * wrapper commands pass normal positional command tokens.
 */
TArray<FString> BuildCliArgTokens(const FString &Params) {
  return BuildCliArgTokensRecursive(Params, 0, TArray<FString>());
}

FCliInvocation NormalizeCliInvocationRecursive(
    const TArray<FString> &RawTokens, int32 Index, FCliInvocation Invocation) {
  const FString Token =
      Index < RawTokens.Num() ? RawTokens[Index] : FString(TEXT(""));
  return Index >= RawTokens.Num()
             ? Invocation
             : ((Token == TEXT("--api-url") || Token == TEXT("--apiUrl")) &&
                Index + 1 < RawTokens.Num())
                   ? (Invocation.ApiUrl = RawTokens[Index + 1],
                      NormalizeCliInvocationRecursive(
                          RawTokens, Index + 2, MoveTemp(Invocation)))
                   : Token.StartsWith(TEXT("--api-url="))
                         ? (Invocation.ApiUrl =
                                Token.Mid(FString(TEXT("--api-url=")).Len()),
                            NormalizeCliInvocationRecursive(
                                RawTokens, Index + 1, MoveTemp(Invocation)))
                         : Token.StartsWith(TEXT("--apiUrl="))
                               ? (Invocation.ApiUrl = Token.Mid(
                                      FString(TEXT("--apiUrl=")).Len()),
                                  NormalizeCliInvocationRecursive(
                                      RawTokens, Index + 1,
                                      MoveTemp(Invocation)))
                               : ((Token == TEXT("--api-key") ||
                                   Token == TEXT("--apiKey")) &&
                                  Index + 1 < RawTokens.Num())
                                     ? (Invocation.ApiKey =
                                            RawTokens[Index + 1],
                                        NormalizeCliInvocationRecursive(
                                            RawTokens, Index + 2,
                                            MoveTemp(Invocation)))
                                     : Token.StartsWith(TEXT("--api-key="))
                                           ? (Invocation.ApiKey = Token.Mid(
                                                  FString(TEXT("--api-key="))
                                                      .Len()),
                                              NormalizeCliInvocationRecursive(
                                                  RawTokens, Index + 1,
                                                  MoveTemp(Invocation)))
                                           : Token.StartsWith(TEXT("--apiKey="))
                                                 ? (Invocation.ApiKey =
                                                        Token.Mid(FString(
                                                                      TEXT("--apiKey="))
                                                                      .Len()),
                                                    NormalizeCliInvocationRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Invocation)))
                                                 : (Invocation.Tokens.Add(
                                                        Token),
                                                    NormalizeCliInvocationRecursive(
                                                        RawTokens, Index + 1,
                                                        MoveTemp(Invocation)));
}

FCliInvocation NormalizeCliInvocation(const TArray<FString> &RawTokens) {
  return NormalizeCliInvocationRecursive(RawTokens, 0, FCliInvocation());
}

/**
 * Builds positional command arguments from raw Unreal commandlet params.
 * User Story: As command dispatch, I need raw commandlet params converted into
 * handler-friendly args so CLI routing can reuse the shared command handlers.
 */
TArray<FString> BuildCommandArgs(const FString &Command,
                                 const FString &Params) {
  return func::or_else(
      func::multi_match<FString, TArray<FString>>(
          Command,
          {
              /**
               * ---- NPC ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_create")),
                  [&Params](const FString &) {
                    TArray<FString> A;
                    A.Add(ExtractParam(Params, TEXT("Persona=")));
                    return A;
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_process")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Id="), TEXT("Input=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("npc_update")),
                  [&Params](const FString &) {
                    return BuildParams(
                        Params,
                        {TEXT("Id="), TEXT("Mood="), TEXT("Inventory=")});
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

              /**
               * ---- Memory (remote) ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
              func::when<FString, TArray<FString>>(
                  [](const FString &C) {
                    return C == TEXT("memory_list") ||
                           C == TEXT("memory_clear") ||
                           C == TEXT("memory_export");
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
                    return BuildParams(Params, {TEXT("Id="), TEXT("Obs=")});
                  }),

              /**
                                      {TEXT("Id="), TEXT("Prompt=")});
                  }),

              /**
               * ---- Ghost ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("ghost_run")),
                  [&Params](const FString &) {
                    return BuildParams(Params,
                                      {TEXT("Suite="), TEXT("Duration=")});
                  }),
              func::when<FString, TArray<FString>>(
                  [](const FString &C) {
                    return C == TEXT("ghost_status") ||
                           C == TEXT("ghost_results") ||
                           C == TEXT("ghost_stop");
                  },
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("SessionId=")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("ghost_history")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Limit=")});
                  }),

              /**
               * ---- Bridge ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
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

              /**
               * ---- Rules ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
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

              /**
               * ---- Soul ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("soul_export")),
                  [&Params](const FString &) {
                    return BuildParams(Params, {TEXT("Id=")});
                  }),
              func::when<FString, TArray<FString>>(
                  [](const FString &C) {
                    return C == TEXT("soul_import") ||
                           C == TEXT("soul_import_npc") ||
                           C == TEXT("soul_verify");
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

              /**
               * ---- Config ----
               * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
               */
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

              /**
               * ---- Setup ----
               * User Story: As setup command routing, I need commandlet params converted
               * into setup flags so the commandlet path matches the direct CLI surface.
               */
              func::when<FString, TArray<FString>>(
                  [](const FString &C) {
                    return C == TEXT("setup") || C == TEXT("setup_deps");
                  },
                  [&Params](const FString &) {
                    return BuildFlags(
                        Params,
                        {TEXT("SqliteOnly")},
                        {TEXT("--sqlite-only")});
                  }),
              func::when<FString, TArray<FString>>(
                  func::equals<FString>(TEXT("setup_runtime_check")),
                  [&Params](const FString &) {
                    return MergeArgs(
                        BuildPrefixed(
                            Params,
                            {TEXT("Database=")},
                            {TEXT("--database=")}),
                        BuildFlags(
                            Params,
                            {TEXT("SkipVector"), TEXT("SkipMemory"),
                             TEXT("Cleanup")},
                            {TEXT("--skip-vector"), TEXT("--skip-memory"),
                             TEXT("--cleanup")}));
                  }),
          }),
      TArray<FString>());
}

} // namespace

UForbocAICommandlet::UForbocAICommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/**
 * Runs the commandlet entrypoint for the requested CLI command.
 * User Story: As Unreal CLI execution, I need one main entrypoint so command
 * parameters can be validated and dispatched consistently.
 */
int32 UForbocAICommandlet::Main(const FString &Params) {
  SDKConfig::InitializeConfig();

  FString Command;
  FParse::Value(*Params, TEXT("Command="), Command);

  const FCliInvocation CliInvocation =
      NormalizeCliInvocation(BuildCliArgTokens(Params));
  const ForbocAI::CLI::FCommandParseResult ParsedCli =
      ForbocAI::CLI::ParseCommandTokens(CliInvocation.Tokens);
  const bool bUseCliTokens = Command.IsEmpty() && ParsedCli.bMatched;
  const FString EffectiveCommand =
      !Command.IsEmpty()
          ? Command
          : bUseCliTokens
                ? ParsedCli.CommandKey
                : (CliInvocation.Tokens.Num() > 0
                       ? CliInvocation.Tokens[0]
                       : FString(TEXT("")));
  const TArray<FString> Args =
      bUseCliTokens ? ParsedCli.Args : BuildCommandArgs(EffectiveCommand, Params);

  FString ApiUrl;
  FParse::Value(*Params, TEXT("ApiUrl="), ApiUrl);

  FString ApiKey;
  FParse::Value(*Params, TEXT("ApiKey="), ApiKey);

  const FString EffectiveApiUrl =
      !ApiUrl.IsEmpty() ? ApiUrl : CliInvocation.ApiUrl;
  const FString EffectiveApiKey =
      !ApiKey.IsEmpty() ? ApiKey : CliInvocation.ApiKey;

  (!EffectiveApiUrl.IsEmpty() || !EffectiveApiKey.IsEmpty())
      ? (SDKConfig::SetApiConfig(
             EffectiveApiUrl.IsEmpty() ? SDKConfig::GetApiUrl()
                                       : EffectiveApiUrl,
             EffectiveApiKey.IsEmpty() ? SDKConfig::GetApiKey()
                                       : EffectiveApiKey),
         void())
      : void();

  UE_LOG(LogTemp, Display, TEXT("ForbocAI CLI (UE5) - Command: %s"),
         *EffectiveCommand);

  bool bCommandFailed = false;
  FString CommandError;
  createCommandPipeline(EffectiveCommand, Args)
      .then([]() {
        UE_LOG(LogTemp, Display, TEXT(""));
        UE_LOG(LogTemp, Display,
               TEXT("[RESULT] Command completed successfully"));
      })
      .catch_([&bCommandFailed, &CommandError](std::string Message) {
        bCommandFailed = true;
        CommandError = UTF8_TO_TCHAR(Message.c_str());
        UE_LOG(LogTemp, Error, TEXT(""));
        UE_LOG(LogTemp, Error, TEXT("[RESULT] Command failed: %s"),
               *CommandError);
      })
      .execute();
  return bCommandFailed ? 1 : 0;
}

UForbocAICommandlet::CommandResult
UForbocAICommandlet::executeCommand(const FString &Command,
                                    const TArray<FString> &Args) {
  return CLIOps::DispatchCommand(Command, Args);
}

UForbocAICommandlet::CommandExecution
UForbocAICommandlet::createCommandPipeline(const FString &Command,
                                           const TArray<FString> &Args) {
  return UForbocAICommandlet::CommandExecution::create(
      [this, Command, Args](std::function<void()> Resolve,
                            std::function<void(std::string)> Reject) {
        auto RejectFString = [&Reject](const FString &Error) {
          Reject(TCHAR_TO_UTF8(*Error));
        };
        auto RejectStd = [&Reject](const std::string &Error) {
          Reject(Error);
        };
        (void)RejectStd; // Available for std::string call sites

        const auto Validation =
            func::runValidation(commandValidationPipeline(), Command);
        func::ematch(
            Validation,
            [&RejectFString](const FString &Err) { RejectFString(Err); },
            [this, &Command, &Args, &Resolve,
             &RejectFString, &Reject](const FString &) {
              const CommandResult Result = executeCommand(Command, Args);
              const FString ResultMessage =
                  Result.message.empty()
                      ? FString()
                      : FString(UTF8_TO_TCHAR(Result.message.c_str()));
              Result.isSuccessful()
                  ? (Resolve(), void())
                  : (RejectFString(
                         ResultMessage.IsEmpty()
                             ? FString::Printf(TEXT("Command failed: %s"),
                                               *Command)
                             : ResultMessage),
                     void());
            });
      });
}

CLITypes::ValidationPipeline<FString, FString>
UForbocAICommandlet::commandValidationPipeline() {
  return func::validationPipeline<FString, FString>() |
         [](const FString &Command) -> CLITypes::Either<FString, FString> {
           return Command.IsEmpty()
                      ? CLITypes::make_left(
                            FString(TEXT("Command cannot be empty")), FString())
                      : CLITypes::make_right(FString(), Command);
         } |
         [](const FString &Command) -> CLITypes::Either<FString, FString> {
           return !ForbocAI::CLI::IsValidCommandKey(Command)
                      ? CLITypes::make_left(
                            FString::Printf(TEXT("Invalid command: %s"),
                                             *Command),
                            FString())
                      : CLITypes::make_right(FString(), Command);
         };
}
