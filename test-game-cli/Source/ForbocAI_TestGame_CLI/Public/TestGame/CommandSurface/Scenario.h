#pragma once

#include "CoreMinimal.h"
#include "TestGame/CommandSurface/Execute.h"

namespace TestGame {
namespace CommandSurface {

inline TArray<FTranscriptEntry> RunScenarioRecursive(
    const FScenarioStep &Scenario, FAliasState &Aliases, int32 CmdIdx,
    TArray<FTranscriptEntry> Acc) {
  return CmdIdx >= Scenario.Commands.Num()
             ? Acc
             : (Acc.Add(
                    ExecuteSpec(Scenario.Commands[CmdIdx], Scenario.Id, Aliases)),
                RunScenarioRecursive(Scenario, Aliases, CmdIdx + 1, Acc));
}

inline TArray<FTranscriptEntry> RunAllScenarios(
    const TArray<FScenarioStep> &Steps, FAliasState &Aliases, int32 StepIdx = 0,
    TArray<FTranscriptEntry> Acc = {}) {
  return StepIdx >= Steps.Num()
             ? Acc
             : [&]() {
                 UE_LOG(LogTemp, Display,
                        TEXT("CommandSurface: Running scenario [%d/%d] '%s'"),
                        StepIdx + 1, Steps.Num(), *Steps[StepIdx].Title);
                 TArray<FTranscriptEntry> StepEntries =
                     RunScenarioRecursive(Steps[StepIdx], Aliases, 0, {});
                 const auto AppendRecursive =
                     [&Acc, &StepEntries](int32 Idx, const auto &Self) -> void {
                   return Idx >= StepEntries.Num()
                              ? void()
                              : (Acc.Add(StepEntries[Idx]),
                                 Self(Idx + 1, Self));
                 };
                 AppendRecursive(0, AppendRecursive);
                 return RunAllScenarios(Steps, Aliases, StepIdx + 1, Acc);
               }();
}

} // namespace CommandSurface
} // namespace TestGame
