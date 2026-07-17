#pragma once

#include "CLI/CLIModule.h"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Contract/ContractThunks.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/Alias/AliasAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"

namespace TestGame {
namespace CommandRunner {

namespace detail {

/** User Story: As a command-runner consumer, I need malformed roots represented by one authored failure value so routing never falls through to another command family. @fn inline FCommandOutput InvalidCommandOutput() */
inline FCommandOutput InvalidCommandOutput() {
  return FCommandOutput{
      GameAdapters::GameRuntimeData().statuses.error,
      CommandRunnerData().syntax.invalidCommandMessage, FString(),
      FCommandAliasUpdate()};
}

/** User Story: As a test-game CLI verifier, I need the contract command executed through the same RTK Query transport used by explicit contract invocations. @fn inline FCommandOutput ExecuteTestGameCommand(const TArray<FString> &Tokens, FTestGameStore &Store, const FString &ApiUrl) */
inline FCommandOutput ExecuteTestGameCommand(const TArray<FString> &Tokens,
                                             FTestGameStore &Store,
                                             const FString &ApiUrl) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Tokens.Num() != Data.limits.testGameCommandTokenCount ||
                 Tokens[Data.limits.testGameCommandTokenIndex] !=
                     Data.commands.contract
             ? InvalidCommandOutput()
             : [&]() {
                 const Contract::FContractQueryResult Result =
                     Contract::queryContractJson(Store, ApiUrl);
                 return FCommandOutput{
                     Result.bSuccess
                         ? GameAdapters::GameRuntimeData().statuses.ok
                         : GameAdapters::GameRuntimeData().statuses.error,
                     Result.bSuccess ? Result.Body : Result.Error,
                     Data.commands.contract, FCommandAliasUpdate()};
               }();
}

/** User Story: As a test-game CLI verifier, I need SDK commands dispatched only after their authored root is matched so the harness tests the public CLI boundary explicitly. @fn inline FCommandOutput ExecuteSdkCommand(const FString &CommandText, const TArray<FString> &Tokens, const FCommandAliasState &Aliases) */
inline FCommandOutput ExecuteSdkCommand(const FString &CommandText,
                                        const TArray<FString> &Tokens,
                                        const FCommandAliasState &Aliases) {
  const FString CommandKey = MapToCommandKey(Tokens);
  const TArray<FString> RawArgs = ExtractArgs(Tokens);
  return func::match(
      FindUnresolvedCommandAlias(CommandKey, RawArgs, Aliases),
      [&CommandKey, &CommandText](const FString &Alias) {
        return FCommandOutput{
            GameAdapters::GameRuntimeData().statuses.error,
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
                ? GameAdapters::GameRuntimeData().statuses.ok
                : GameAdapters::GameRuntimeData().statuses.error,
            Message, CommandKey, AliasUpdate};
      });
}

} // namespace detail

/** User Story: As a systems harness command runner consumer, I need authored command roots routed through their actual CLI handlers so contract and SDK coverage share one execution boundary. @fn inline FCommandOutput Execute(const FString &CommandText, FTestGameStore &Store, const FString &ApiUrl, const FCommandAliasState &Aliases) */
inline FCommandOutput Execute(const FString &CommandText,
                              FTestGameStore &Store,
                              const FString &ApiUrl,
                              const FCommandAliasState &Aliases) {
  const FCommandRunnerData &Data = CommandRunnerData();
  const TArray<FString> Tokens = detail::Tokenize(CommandText);
  return Tokens.Num() < Data.limits.rootTokenCount
             ? detail::InvalidCommandOutput()
         : Tokens[Data.limits.firstTokenIndex] ==
                   Data.syntax.testGameRootCommand
             ? detail::ExecuteTestGameCommand(Tokens, Store, ApiUrl)
         : Tokens[Data.limits.firstTokenIndex] == Data.syntax.sdkRootCommand
             ? detail::ExecuteSdkCommand(CommandText, Tokens, Aliases)
             : detail::InvalidCommandOutput();
}

} // namespace CommandRunner
} // namespace TestGame
