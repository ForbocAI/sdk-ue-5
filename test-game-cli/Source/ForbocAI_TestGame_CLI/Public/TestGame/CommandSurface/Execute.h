#pragma once

#include "CLI/CliOperations.h"
#include "Core/ue_fp.hpp"
#include "RuntimeStore.h"
#include "TestGame/CommandSurface/Alias.h"
#include "TestGame/CommandSurface/Tokens.h"

namespace TestGame {
namespace CommandSurface {

inline FCommandOutput Execute(const FString &CommandText,
                              FAliasState &Aliases) {
  const TArray<FString> Tokens = detail::Tokenize(CommandText);
  if (Tokens.Num() < 1 || Tokens[0] != TEXT("forbocai")) {
    return FCommandOutput{ETranscriptStatus::Error,
                          TEXT("Not a forbocai command"), TEXT("")};
  }

  const FString CommandKey = detail::MapToCommandKey(Tokens);
  TArray<FString> Args = detail::ExtractArgs(Tokens);
  (Args.Num() > 0 &&
   (CommandKey.Contains(TEXT("npc_")) || CommandKey.Contains(TEXT("memory_")) ||
    CommandKey.Contains(TEXT("soul_"))))
      ? (Args[0] = detail::ResolveNpcAlias(Aliases, Args[0]), void())
      : void();

  (CommandKey == TEXT("bridge_validate") && Args.Num() > 0 &&
   Aliases.BridgeValidateCommandRule == TEXT("expand_preset_macro"))
      ? (Args[0] = detail::ExpandBridgePreset(Aliases, Args[0]), void())
      : void();

  const func::TestResult<void> Result =
      CLIOps::DispatchCommand(CommandKey, Args);
  const FString ResultMessage =
      Result.message.empty() ? FString()
                             : FString(UTF8_TO_TCHAR(Result.message.c_str()));

  (CommandKey == TEXT("npc_create") && Result.bSuccess && Args.Num() > 0 &&
   Aliases.NpcCreateAliasRule == TEXT("substitute_generated_npc_id"))
      ? (Aliases.NpcAliases.Add(Args[0], ResultMessage), void())
      : void();

  return FCommandOutput{
      Result.bSuccess ? ETranscriptStatus::Ok : ETranscriptStatus::Error,
      ResultMessage, CommandKey};
}

inline FTranscriptEntry ExecuteSpec(const FCommandSpec &Spec,
                                    const FString &ScenarioId,
                                    FAliasState &Aliases) {
  const FCommandOutput Output = Execute(Spec.Command, Aliases);
  FTranscriptEntry Entry;
  Entry.Id = FGuid::NewGuid().ToString();
  Entry.ScenarioId = ScenarioId;
  Entry.CommandGroup = Spec.Group;
  Entry.Command = Spec.Command;
  Entry.ExpectedRoutes = Spec.ExpectedRoutes;
  Entry.Status = Output.Status;
  Entry.Output = Output.Output;
  Entry.Timestamp = FDateTime::UtcNow().ToString();
  return Entry;
}

} // namespace CommandSurface
} // namespace TestGame
