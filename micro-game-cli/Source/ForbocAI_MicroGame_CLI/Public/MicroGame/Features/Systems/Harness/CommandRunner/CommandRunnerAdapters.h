#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Configuration/ConfigurationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"

namespace MicroGame {
namespace CommandRunner {

namespace detail {

/** User Story: As a systems harness command runner consumer, I need to invoke resolve alias value through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline FString ResolveAliasValue(const TMap<FString, FString> &Aliases, const FString &Candidate) */
inline FString ResolveAliasValue(const TMap<FString, FString> &Aliases,
                                 const FString &Candidate) {
  return func::map_value_or<FString, FString>(Aliases, Candidate, Candidate);
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
  const FGameRuntimeData &Data = VerificationVocabularyAdapters::GameRuntimeData();
  TMap<FString, FString> Values;
  Values.Add(Data.tokens.command, Command.Command);
  Values.Add(Data.tokens.value, Assertion.Value);
  Values.Add(Data.tokens.kind, Assertion.Kind);
  Values.Add(Data.tokens.alias, Assertion.Value);
  return !Data.outputAssertionKinds.all.Contains(Assertion.Kind)
             ? func::just(VerificationAdapters::FormatGameTemplate(
                   Data.messages.outputAssertionKindUnsupported, Values))
         : Assertion.Kind == Data.outputAssertionKinds.excludesText
             ? (Result.Output.Contains(Assertion.Value)
                    ? func::just(VerificationAdapters::FormatGameTemplate(
                          Data.messages.outputAssertionValuePresent,
                          Values))
                    : func::nothing<FString>())
         : Assertion.Kind == Data.outputAssertionKinds.includesText
             ? (Result.Output.Contains(Assertion.Value)
                    ? func::nothing<FString>()
                    : func::just(VerificationAdapters::FormatGameTemplate(
                          Data.messages.outputAssertionValueMissing,
                          Values)))
             : func::match(
                   ResolveOutputAssertionAlias(Assertion.Value, Aliases),
                   [&Data, &Result, &Values](const FString &Expected) {
                     return Result.Output.Contains(Expected)
                                ? func::nothing<FString>()
                                : func::just(
                                      VerificationAdapters::FormatGameTemplate(
                                          Data.messages
                                              .outputAssertionValueMissing,
                                          Values));
                   },
                   [&Data, &Values]() {
                     return func::just(VerificationAdapters::FormatGameTemplate(
                         Data.messages.outputAssertionAliasMissing, Values));
                   });
}

/**
 * User Story: As a systems harness command runner consumer, I need output assertions evaluated recursively through a stable signature so every authored assertion contributes to command status.
 * @fn inline FCommandOutput ValidateOutputAssertionsRecursive( const FCommandSpec &Command, const FCommandOutput &Result, const FCommandAliasState &Aliases, int32 Index)
 */
inline FCommandOutput ValidateOutputAssertionsRecursive(
    const FCommandSpec &Command, const FCommandOutput &Result,
    const FCommandAliasState &Aliases, int32 Index) {
  const FGameRuntimeData &Data = VerificationVocabularyAdapters::GameRuntimeData();
  return Result.Status == Data.statuses.error ||
                 Index >= Command.OutputAssertions.Num()
             ? Result
             : func::match(
                   OutputAssertionFailureReason(
                       Command, Command.OutputAssertions[Index], Result,
                       Aliases),
                   [&Result](const FString &Reason) {
                     const FGameRuntimeData &Data =
                         VerificationVocabularyAdapters::GameRuntimeData();
                     TMap<FString, FString> Values;
                     Values.Add(Data.tokens.reason, Reason);
                     Values.Add(Data.tokens.output, Result.Output);
                     return FCommandOutput{
                         Data.statuses.error,
                         VerificationAdapters::FormatGameTemplate(
                             Data.messages.outputAssertionFailure, Values),
                         Result.RoutedThrough, Result.AliasUpdate};
                   },
                   [&Command, &Result, &Aliases, &Data, Index]() {
                     return ValidateOutputAssertionsRecursive(
                         Command, Result, Aliases,
                         Index + Data.numbers.nextIndex);
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
} // namespace MicroGame
