#pragma once

#include "CLI/CLIModule.h"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Harness/CommandRunner/Alias/AliasAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"

namespace TestGame {
namespace CommandRunner {

/** User Story: As a systems harness command runner consumer, I need to invoke execute through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FCommandOutput Execute(const FString &CommandText, const FCommandAliasState &Aliases) */
inline FCommandOutput Execute(const FString &CommandText,
                              const FCommandAliasState &Aliases) {
  const FCommandRunnerData &Data = CommandRunnerData();
  const TArray<FString> Tokens = detail::Tokenize(CommandText);
  return Tokens.Num() < Data.limits.rootTokenCount ||
                 Tokens[Data.limits.firstTokenIndex] !=
                     Data.syntax.rootCommand
             ? FCommandOutput{ETranscriptStatus::Error,
                              Data.syntax.invalidCommandMessage, FString(),
                              FCommandAliasUpdate()}
             : [&]() {
                 const FString CommandKey = detail::MapToCommandKey(Tokens);
                 const TArray<FString> RawArgs = detail::ExtractArgs(Tokens);
                 return func::match(
                     FindUnresolvedCommandAlias(CommandKey, RawArgs,
                                                Aliases),
                     [&CommandText, &CommandKey](const FString &Alias) {
                       return FCommandOutput{
                           ETranscriptStatus::Error,
                           FormatUnresolvedCommandAlias(CommandText, Alias),
                           CommandKey, FCommandAliasUpdate()};
                     },
                     [&Aliases, &CommandKey, &RawArgs]() {
                       const TArray<FString> Args =
                           ResolveCommandAliasTokens(CommandKey, RawArgs,
                                                     Aliases);
                       const func::TestResult<void> Result =
                           CLIOps::DispatchCommand(CommandKey, Args);
                       const FString Message =
                           Result.message.empty()
                               ? FString()
                               : FString(
                                     UTF8_TO_TCHAR(Result.message.c_str()));
                       const FCommandAliasUpdate AliasUpdate =
                           ReadCommandAliasUpdate(CommandKey, RawArgs,
                                                  Message, Result.bSuccess);
                       return FCommandOutput{
                           Result.bSuccess ? ETranscriptStatus::Ok
                                           : ETranscriptStatus::Error,
                           Message, CommandKey, AliasUpdate};
                     });
               }();
}

} // namespace CommandRunner
} // namespace TestGame
