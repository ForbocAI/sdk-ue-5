#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Harness/CommandRunner/Configuration/ConfigurationAdapters.h"

namespace TestGame {
namespace CommandRunner {

namespace detail {

/** User Story: As a systems harness command runner consumer, I need to invoke resolve alias value through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FString ResolveAliasValue(const TMap<FString, FString> &Aliases, const FString &Candidate) */
inline FString ResolveAliasValue(const TMap<FString, FString> &Aliases,
                                 const FString &Candidate) {
  return func::map_value_or<FString, FString>(Aliases, Candidate, Candidate);
}

/** User Story: As a systems harness command runner consumer, I need to invoke map to command key through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FString MapToCommandKey(const TArray<FString> &Tokens) */
inline FString MapToCommandKey(const TArray<FString> &Tokens) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Tokens.Num() < Data.limits.domainTokenCount
             ? Data.syntax.unknownCommandKey
         : Tokens.Num() < Data.limits.commandTokenCount
             ? Tokens[Data.limits.domainTokenIndex]
             : Tokens[Data.limits.domainTokenIndex] +
                   Data.syntax.commandSeparator +
                   Tokens[Data.limits.actionTokenIndex];
}

/** User Story: As a systems harness command runner consumer, I need to invoke extract args recursive through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TArray<FString> ExtractArgsRecursive(const TArray<FString> &Tokens, int32 Index, TArray<FString> Acc) */
inline TArray<FString> ExtractArgsRecursive(const TArray<FString> &Tokens,
                                            int32 Index,
                                            TArray<FString> Acc) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Index >= Tokens.Num()
             ? Acc
             : ExtractArgsRecursive(
                   Tokens, Index + Data.limits.nextIndex,
                   Tokens[Index] == Data.syntax.textOption
                       ? MoveTemp(Acc)
                       : func::append_value<FString>(MoveTemp(Acc),
                                                     Tokens[Index]));
}

/** User Story: As a systems harness command runner consumer, I need to invoke extract args through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TArray<FString> ExtractArgs(const TArray<FString> &Tokens) */
inline TArray<FString> ExtractArgs(const TArray<FString> &Tokens) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Tokens.Num() <= Data.limits.argumentStartIndex
             ? TArray<FString>()
             : ExtractArgsRecursive(Tokens,
                                    Data.limits.argumentStartIndex,
                                    TArray<FString>());
}

/** User Story: As a systems harness command runner consumer, I need to invoke is quote character through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline bool IsQuoteCharacter(const TCHAR Character) */
inline bool IsQuoteCharacter(const TCHAR Character) {
  const FCommandRunnerQuotes &Quotes = CommandRunnerData().quotes;
  const int32 FirstCharacterIndex =
      CommandRunnerData().limits.firstTokenIndex;
  return Character == Quotes.doubleQuote[FirstCharacterIndex] ||
         Character == Quotes.singleQuote[FirstCharacterIndex];
}

/** User Story: As a systems harness command runner consumer, I need to invoke tokenize recursive through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TArray<FString> TokenizeRecursive(const FString &Command, int32 Index, TCHAR QuoteCharacter, FString Current, TArray<FString> Tokens) */
inline TArray<FString> TokenizeRecursive(const FString &Command, int32 Index,
                                         TCHAR QuoteCharacter,
                                         FString Current,
                                         TArray<FString> Tokens) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Index >= Command.Len()
             ? (Current.IsEmpty()
                    ? Tokens
                    : func::append_value<FString>(MoveTemp(Tokens), Current))
         : QuoteCharacter != TCHAR() && Command[Index] == QuoteCharacter
             ? TokenizeRecursive(Command, Index + Data.limits.nextIndex,
                                 TCHAR(), MoveTemp(Current), MoveTemp(Tokens))
         : QuoteCharacter == TCHAR() && IsQuoteCharacter(Command[Index])
             ? TokenizeRecursive(Command, Index + Data.limits.nextIndex,
                                 Command[Index], MoveTemp(Current),
                                 MoveTemp(Tokens))
         : QuoteCharacter == TCHAR() && FChar::IsWhitespace(Command[Index])
             ? TokenizeRecursive(
                   Command, Index + Data.limits.nextIndex, TCHAR(), FString(),
                   Current.IsEmpty()
                       ? MoveTemp(Tokens)
                       : func::append_value<FString>(MoveTemp(Tokens), Current))
             : TokenizeRecursive(
                   Command, Index + Data.limits.nextIndex, QuoteCharacter,
                   Current + FString::Chr(Command[Index]), MoveTemp(Tokens));
}

/** User Story: As a systems harness command runner consumer, I need to invoke tokenize through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TArray<FString> Tokenize(const FString &Command) */
inline TArray<FString> Tokenize(const FString &Command) {
  return TokenizeRecursive(Command,
                           CommandRunnerData().limits.firstTokenIndex,
                           TCHAR(), FString(), TArray<FString>());
}

/** User Story: As a systems harness command runner consumer, I need to invoke resolves npc alias through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline bool ResolvesNpcAlias(const FString &CommandKey) */
inline bool ResolvesNpcAlias(const FString &CommandKey) {
  const FCommandRunnerCommands &Commands = CommandRunnerData().commands;
  return CommandKey.StartsWith(Commands.npcPrefix) ||
         CommandKey.StartsWith(Commands.memoryPrefix) ||
         (CommandKey.StartsWith(Commands.soulPrefix) &&
          CommandKey != Commands.soulImport);
}

} // namespace detail

/** User Story: As a systems harness command runner consumer, I need to invoke resolve command alias tokens through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TArray<FString> ResolveCommandAliasTokens( const FString &CommandKey, const TArray<FString> &Args, const FCommandAliasState &Aliases) */
inline TArray<FString> ResolveCommandAliasTokens(
    const FString &CommandKey, const TArray<FString> &Args,
    const FCommandAliasState &Aliases) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Args.Num() <= Data.limits.firstArgumentIndex
             ? Args
             : [&]() {
                 TArray<FString> Resolved = Args;
                 const FString &Candidate =
                     Args[Data.limits.firstArgumentIndex];
                 Resolved[Data.limits.firstArgumentIndex] =
                     CommandKey == Data.commands.soulImport &&
                             Candidate == Data.aliases.soulTransaction
                         ? detail::ResolveAliasValue(
                               Aliases.SoulTransactionAliases, Candidate)
                     : CommandKey.StartsWith(Data.commands.ghostPrefix) &&
                               CommandKey != Data.commands.ghostRun &&
                               Candidate == Data.aliases.ghostSession
                         ? detail::ResolveAliasValue(
                               Aliases.GhostSessionAliases, Candidate)
                     : detail::ResolvesNpcAlias(CommandKey)
                         ? detail::ResolveAliasValue(Aliases.NpcAliases,
                                                     Candidate)
                         : Candidate;
                 return Resolved;
               }();
}

/** User Story: As a systems harness command runner consumer, I need to invoke create command alias state through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FCommandAliasState CreateCommandAliasState( const TMap<FString, FString> &NpcAliases, const TMap<FString, FString> &GhostSessionAliases, const TMap<FString, FString> &SoulTransactionAliases) */
inline FCommandAliasState CreateCommandAliasState(
    const TMap<FString, FString> &NpcAliases,
    const TMap<FString, FString> &GhostSessionAliases,
    const TMap<FString, FString> &SoulTransactionAliases) {
  return FCommandAliasState{NpcAliases, GhostSessionAliases,
                            SoulTransactionAliases};
}

/** User Story: As a systems harness command runner consumer, I need to invoke read command alias update through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FCommandAliasUpdate ReadCommandAliasUpdate( const FString &CommandKey, const TArray<FString> &RawArgs, const FString &Output, bool bSuccess) */
inline FCommandAliasUpdate ReadCommandAliasUpdate(
    const FString &CommandKey, const TArray<FString> &RawArgs,
    const FString &Output, bool bSuccess) {
  const FCommandRunnerData &Data = CommandRunnerData();
  const bool bHasIdentifier =
      bSuccess && RawArgs.Num() > Data.limits.firstArgumentIndex &&
      !Output.IsEmpty();
  FCommandAliasUpdate Update;
  Update.NpcAliases =
      bHasIdentifier && CommandKey == Data.commands.npcCreate
          ? func::upsert_map_value<FString, FString>(
                TMap<FString, FString>(),
                RawArgs[Data.limits.firstArgumentIndex], FString(),
                [&Output](const FString &) { return Output; })
          : TMap<FString, FString>();
  Update.GhostSessionAliases =
      bSuccess && CommandKey == Data.commands.ghostRun && !Output.IsEmpty()
          ? func::upsert_map_value<FString, FString>(
                TMap<FString, FString>(), Data.aliases.ghostSession,
                FString(), [&Output](const FString &) { return Output; })
          : TMap<FString, FString>();
  Update.SoulTransactionAliases =
      bSuccess && CommandKey == Data.commands.soulExport && !Output.IsEmpty()
          ? func::upsert_map_value<FString, FString>(
                TMap<FString, FString>(),
                Data.aliases.soulTransaction, FString(),
                [&Output](const FString &) { return Output; })
          : TMap<FString, FString>();
  return Update;
}

/** User Story: As a systems harness command runner consumer, I need to invoke has command alias update through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline bool HasCommandAliasUpdate(const FCommandAliasUpdate &Update) */
inline bool HasCommandAliasUpdate(const FCommandAliasUpdate &Update) {
  return Update.NpcAliases.Num() > CommandRunnerData().limits.firstTokenIndex ||
         Update.GhostSessionAliases.Num() >
             CommandRunnerData().limits.firstTokenIndex ||
         Update.SoulTransactionAliases.Num() >
             CommandRunnerData().limits.firstTokenIndex;
}

/** User Story: As a systems harness command runner consumer, I need assertion aliases resolved from the single root-store alias state through a stable signature so validation and command execution share identity. @fn inline func::Maybe<FString> ResolveOutputAssertionAlias(const FString &Alias, const FCommandAliasState &Aliases) */
inline func::Maybe<FString>
ResolveOutputAssertionAlias(const FString &Alias,
                            const FCommandAliasState &Aliases) {
  const func::Maybe<FString> Npc =
      func::find_map_value<FString, FString>(Aliases.NpcAliases, Alias);
  const func::Maybe<FString> Ghost = func::find_map_value<FString, FString>(
      Aliases.GhostSessionAliases, Alias);
  return Npc.hasValue
             ? Npc
             : Ghost.hasValue
                   ? Ghost
                   : func::find_map_value<FString, FString>(
                         Aliases.SoulTransactionAliases, Alias);
}

/**
 * User Story: As a systems harness command runner consumer, I need each semantic assertion reduced to an optional failure reason through a stable signature so validation remains pure and composable.
 * @fn inline func::Maybe<FString> OutputAssertionFailureReason( const FCommandSpec &Command, const FOutputAssertion &Assertion, const FCommandOutput &Result, const FCommandAliasState &Aliases)
 */
inline func::Maybe<FString> OutputAssertionFailureReason(
    const FCommandSpec &Command, const FOutputAssertion &Assertion,
    const FCommandOutput &Result, const FCommandAliasState &Aliases) {
  const FCommandRunnerMessages &Messages = CommandRunnerData().messages;
  return Assertion.Kind != EOutputAssertionKind::IncludesAlias
             ? func::just(FString::Format(
                   *Messages.outputAssertionKindUnsupported,
                   {Command.Command}))
             : func::match(
                   ResolveOutputAssertionAlias(Assertion.Alias, Aliases),
                   [&Command, &Assertion, &Messages,
                    &Result](const FString &Expected) {
                     return Result.Output.Contains(Expected)
                                ? func::nothing<FString>()
                                : func::just(FString::Format(
                                      *Messages.outputAssertionValueMissing,
                                      {Command.Command, Assertion.Alias}));
                   },
                   [&Command, &Assertion, &Messages]() {
                     return func::just(FString::Format(
                         *Messages.outputAssertionAliasMissing,
                         {Command.Command, Assertion.Alias}));
                   });
}

/**
 * User Story: As a systems harness command runner consumer, I need output assertions evaluated recursively through a stable signature so every authored assertion contributes to command status.
 * @fn inline FCommandOutput ValidateOutputAssertionsRecursive( const FCommandSpec &Command, const FCommandOutput &Result, const FCommandAliasState &Aliases, int32 Index)
 */
inline FCommandOutput ValidateOutputAssertionsRecursive(
    const FCommandSpec &Command, const FCommandOutput &Result,
    const FCommandAliasState &Aliases, int32 Index) {
  return Result.Status == ETranscriptStatus::Error ||
                 Index >= Command.OutputAssertions.Num()
             ? Result
             : func::match(
                   OutputAssertionFailureReason(
                       Command, Command.OutputAssertions[Index], Result,
                       Aliases),
                   [&Result](const FString &Reason) {
                     return FCommandOutput{
                         ETranscriptStatus::Error,
                         FString::Format(
                             *CommandRunnerData()
                                  .messages.outputAssertionFailure,
                             {Reason, Result.Output}),
                         Result.RoutedThrough, Result.AliasUpdate};
                   },
                   [&Command, &Result, &Aliases, Index]() {
                     return ValidateOutputAssertionsRecursive(
                         Command, Result, Aliases, Index + 1);
                   });
}

/**
 * User Story: As an SDK verifier, I need process success upgraded to semantic success through a stable signature only when every API-owned output assertion passes.
 * @fn inline FCommandOutput ValidateOutputAssertions( const FCommandSpec &Command, const FCommandOutput &Result, const FCommandAliasState &Aliases)
 */
inline FCommandOutput ValidateOutputAssertions(
    const FCommandSpec &Command, const FCommandOutput &Result,
    const FCommandAliasState &Aliases) {
  return ValidateOutputAssertionsRecursive(Command, Result, Aliases,
                                           CommandRunnerData()
                                               .limits.firstTokenIndex);
}

} // namespace CommandRunner
} // namespace TestGame
