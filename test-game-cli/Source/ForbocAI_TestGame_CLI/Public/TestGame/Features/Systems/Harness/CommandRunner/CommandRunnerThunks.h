#pragma once

#include "CLI/CLIModule.h"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace CommandRunner {

inline FCommandOutput Execute(const FString &CommandText,
                              FAliasState &Aliases) {
  const TArray<FString> Tokens = detail::Tokenize(CommandText);
  return Tokens.Num() < 1 || Tokens[0] != TEXT("forbocai")
             ? FCommandOutput{ETranscriptStatus::Error,
                              TEXT("Not a forbocai command"), TEXT("")}
             : [&]() {
                 const FString CommandKey = detail::MapToCommandKey(Tokens);
                 TArray<FString> Args = detail::ExtractArgs(Tokens);
                 (Args.Num() > 0 &&
                  (CommandKey.Contains(TEXT("npc_")) ||
                   CommandKey.Contains(TEXT("memory_")) ||
                   CommandKey.Contains(TEXT("soul_"))))
                     ? (Args[0] =
                            detail::ResolveNpcAlias(Aliases, Args[0]),
                        void())
                     : void();

                 (CommandKey == TEXT("bridge_validate") && Args.Num() > 0 &&
                  Aliases.BridgeValidateCommandRule ==
                      TEXT("expand_preset_macro"))
                     ? (Args[0] =
                            detail::ExpandBridgePreset(Aliases, Args[0]),
                        void())
                     : void();

                 const func::TestResult<void> Result =
                     CLIOps::DispatchCommand(CommandKey, Args);
                 const FString Message =
                     Result.message.empty()
                         ? FString()
                         : FString(
                               UTF8_TO_TCHAR(Result.message.c_str()));

                 (CommandKey == TEXT("npc_create") && Result.bSuccess &&
                  Args.Num() > 0 &&
                  Aliases.NpcCreateAliasRule ==
                      TEXT("substitute_generated_npc_id"))
                     ? (Aliases.NpcAliases.Add(Args[0], Message), void())
                     : void();

                 return FCommandOutput{
                     Result.bSuccess ? ETranscriptStatus::Ok
                                     : ETranscriptStatus::Error,
                     Message, CommandKey};
               }();
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

inline TArray<FTranscriptEntry> RunScenario(
    const FScenarioStep &Scenario, FAliasState &Aliases, int32 CommandIndex,
    TArray<FTranscriptEntry> Acc) {
  return CommandIndex >= Scenario.Commands.Num()
             ? Acc
             : (Acc.Add(ExecuteSpec(Scenario.Commands[CommandIndex],
                                    Scenario.Id, Aliases)),
                RunScenario(Scenario, Aliases, CommandIndex + 1,
                            MoveTemp(Acc)));
}

inline TArray<FTranscriptEntry> RunScenarios(
    const TArray<FScenarioStep> &Steps, FAliasState &Aliases, int32 StepIndex,
    TArray<FTranscriptEntry> Acc) {
  return StepIndex >= Steps.Num()
             ? Acc
             : [&]() {
                 const TArray<FTranscriptEntry> Entries =
                     RunScenario(Steps[StepIndex], Aliases, 0, {});
                 Acc.Append(Entries);
                 return RunScenarios(Steps, Aliases, StepIndex + 1,
                                     MoveTemp(Acc));
               }();
}

} // namespace CommandRunner
} // namespace TestGame
