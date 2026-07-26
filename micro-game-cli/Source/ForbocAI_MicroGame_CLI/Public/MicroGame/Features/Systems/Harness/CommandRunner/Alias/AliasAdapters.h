#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Configuration/ConfigurationAdapters.h"

namespace MicroGame {
namespace CommandRunner {

/**
 * User Story: As an SDK verifier, I need generated identifiers to come from successful prerequisite output so the harness never sends fabricated fixture IDs to a live CLI.
 * @fn inline func::Maybe<FString> FindUnresolvedCommandAlias( const FString &CommandKey, const TArray<FString> &Args, const FCommandAliasState &Aliases)
 */
inline func::Maybe<FString> FindUnresolvedCommandAlias(
    const FString &CommandKey, const TArray<FString> &Args,
    const FCommandAliasState &Aliases) {
  const FCommandRunnerData &Data = CommandRunnerData();
  return Args.Num() <= Data.limits.firstArgumentIndex
             ? func::nothing<FString>()
             : [&]() {
                 const FString &Candidate =
                     Args[Data.limits.firstArgumentIndex];
                 const bool bMissingSoulTransaction =
                     CommandKey == Data.commands.soulImport &&
                     Candidate == Data.aliases.soulTransaction &&
                     !Aliases.SoulTransactionAliases.Contains(Candidate);
                 const bool bMissingGhostSession =
                     CommandKey.StartsWith(Data.commands.ghostPrefix) &&
                     CommandKey != Data.commands.ghostRun &&
                     Candidate == Data.aliases.ghostSession &&
                     !Aliases.GhostSessionAliases.Contains(Candidate);
                 return bMissingSoulTransaction || bMissingGhostSession
                            ? func::just(Candidate)
                            : func::nothing<FString>();
               }();
}

/**
 * User Story: As an SDK verifier, I need unresolved command dependencies reported explicitly so missing live coverage cannot be mistaken for transport against a made-up identifier.
 * @fn inline FString FormatUnresolvedCommandAlias(const FString &CommandText, const FString &Alias)
 */
inline FString FormatUnresolvedCommandAlias(const FString &CommandText,
                                             const FString &Alias) {
  return FString::Format(
      *CommandRunnerData().messages.unresolvedAlias,
      {CommandText, Alias});
}

} // namespace CommandRunner
} // namespace MicroGame
