#pragma once

#include "HAL/PlatformTime.h"
#include "CLI/CLIModule.h"
#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Alias/AliasAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Parsing/CommandRunnerParsingAdapters.h"

namespace MicroGame {
namespace CommandRunner {

namespace detail {

/** User Story: As a command-runner consumer, I need malformed roots represented by one authored failure value so routing never falls through to another command family. @fn inline FCommandOutput InvalidCommandOutput() */
inline FCommandOutput InvalidCommandOutput() {
  return FCommandOutput{
      VerificationVocabularyAdapters::GameRuntimeData().statuses.error,
      CommandRunnerData().syntax.invalidCommandMessage, FString(),
      FCommandAliasUpdate()};
}

/** User Story: As a micro-game CLI verifier, I need SDK commands dispatched only after their authored root is matched so the harness tests the public CLI boundary explicitly. @fn inline FCommandOutput ExecuteSdkCommand(const FString &CommandText, const TArray<FString> &Tokens, const FCommandAliasState &Aliases) */
inline FCommandOutput ExecuteSdkCommand(const FString &CommandText,
                                        const TArray<FString> &Tokens,
                                        const FCommandAliasState &Aliases) {
  const FString CommandKey = Parsing::MapToCommandKey(Tokens);
  const TArray<FString> RawArgs = Parsing::ExtractArgs(Tokens);
  return func::match(
      FindUnresolvedCommandAlias(CommandKey, RawArgs, Aliases),
      [&CommandKey, &CommandText](const FString &Alias) {
        return FCommandOutput{
            VerificationVocabularyAdapters::GameRuntimeData().statuses.error,
            FormatUnresolvedCommandAlias(CommandText, Alias), CommandKey,
            FCommandAliasUpdate()};
      },
      [&Aliases, &CommandKey, &RawArgs]() {
        const TArray<FString> Args =
            ResolveCommandAliasTokens(CommandKey, RawArgs, Aliases);
        const func::TestResult<void> Result =
            CLIOps::DispatchCommand(CommandKey, Args);
        const FString Message =
            Result.message.empty()
                ? FString()
                : FString(UTF8_TO_TCHAR(Result.message.c_str()));
        const FCommandAliasUpdate AliasUpdate = ReadCommandAliasUpdate(
            CommandKey, RawArgs, Message, Result.bSuccess);
        return FCommandOutput{
            Result.bSuccess
                ? VerificationVocabularyAdapters::GameRuntimeData().statuses.ok
                : VerificationVocabularyAdapters::GameRuntimeData().statuses.error,
            Message, CommandKey, AliasUpdate};
      });
}

} // namespace detail

/** User Story: As a systems harness command runner consumer, I need every authored operation routed through the SDK CLI boundary. @fn inline FCommandOutput Execute(const FString &CommandText, const FCommandAliasState &Aliases) */
inline FCommandOutput Execute(const FString &CommandText,
                              const FCommandAliasState &Aliases) {
  const FCommandRunnerData &Data = CommandRunnerData();
  const TArray<FString> Tokens = Parsing::Tokenize(CommandText);
  const double StartedAt = FPlatformTime::Seconds();
  FCommandOutput Result =
      Tokens.Num() < Data.limits.rootTokenCount
          ? detail::InvalidCommandOutput()
      : Tokens[Data.limits.firstTokenIndex] == Data.syntax.sdkRootCommand
          ? detail::ExecuteSdkCommand(CommandText, Tokens, Aliases)
          : detail::InvalidCommandOutput();
  Result.DurationMs =
      (FPlatformTime::Seconds() - StartedAt) *
      static_cast<double>(VerificationAdapters::GameData()
                              .numbers.millisecondsPerSecond);
  return Result;
}

} // namespace CommandRunner
} // namespace MicroGame
