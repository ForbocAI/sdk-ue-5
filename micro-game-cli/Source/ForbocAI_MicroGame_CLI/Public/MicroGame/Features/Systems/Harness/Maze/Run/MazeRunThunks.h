#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Actor/ActorThunks.h"
#include "MicroGame/Features/Systems/Harness/Maze/Run/Turn/MazeRunTurnThunks.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Generation/GenerationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Transcript/Layout/LayoutAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Transcript/Summary/SummaryAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Transcript/TranscriptThunks.h"

namespace MicroGame::Maze {

/**
 * User Story: As Maze transcript assembly, I need topology, turn evidence, outcome, and decision quality projected from one completed run.
 * @fn inline TArray<FString> BuildMazeTranscript( const FMazeNpc &Npc, const FMaze &Maze, const FMazeStep &Step)
 */
inline TArray<FString> BuildMazeTranscript(
    const FMazeNpc &Npc, const FMaze &Maze, const FMazeStep &Step) {
  const FString Summary = Step.bSolved
                              ? FormatMazeSolved(
                                    Step.Turns, Maze.Optimal, Step.DeadEnds)
                              : FormatMazeFailed(
                                    Step.Turns, Step.Cell, Step.DeadEnds,
                                    Maze.Optimal);
  return func::concat_arrays<FString>(
      {{MazeConfig().Header,
        FormatMazeExplorer(Npc.DisplayName, Maze.Seed),
        FormatMazeRunSummary(Step.Turns, Maze.Optimal, Step.DeadEnds,
                             Step.bSolved)},
       FormatMazeLayout(Maze),
       {MazeConfig().RunHeader},
       Step.Lines,
       {Summary,
        FormatMazeDecisionQuality(Step.OptimalChoices, Step.RealChoices),
        MazeConfig().Footer}});
}

/** User Story: As Maze fail-fast evidence, I need an incomplete transcript retained and written instead of a false successful result. @fn inline FMazeRunResult FailMazeRun(const FMazeRunOptions &Options, const FString &Failure) */
inline FMazeRunResult FailMazeRun(const FMazeRunOptions &Options,
                                  const FString &Failure) {
  const TArray<FString> Lines{MazeConfig().Header, Failure,
                              MazeConfig().Footer};
  WriteMazeTranscript(Options.LogTarget, Lines);
  return {false, Lines, Failure};
}

/** User Story: As normal and Ghost Maze modes, I need one shared mechanic parameterized only by its injected CLI command family and log target. @fn inline FMazeRunResult RunMaze(FMicroGameStore &Store, const FMazeCommands &Commands, const FMazeRunOptions &Options) */
inline FMazeRunResult RunMaze(FMicroGameStore &Store,
                              const FMazeCommands &Commands,
                              const FMazeRunOptions &Options) {
  const uint32 Seed = Options.Seed.hasValue ? Options.Seed.value
                                            : CurrentMazeSeed();
  const FMaze Maze = GenerateMaze(Seed);
  const FMazeNpcResult Actor = BuildMazeNpc(Store, Options, Commands);
  return !Actor.bSuccess
             ? FailMazeRun(Options, Actor.Failure)
             : [&]() {
                 const FMazeCommandResult Warm =
                     WarmMazeNpc(Store, Options, Commands, Actor.Npc, Maze);
                 return !Warm.bSuccess
                            ? FailMazeRun(Options, Warm.Failure)
                            : [&]() {
                                const FMazeStep Step = RunMazeTurns(
                                    Store, Options, Commands, Actor.Npc, Maze,
                                    Maze.Start, MazeConfig().Numbers.One,
                                    MazeConfig().Numbers.Zero,
                                    MazeConfig().Numbers.Zero,
                                    MazeConfig().Numbers.Zero, {},
                                    {Maze.Start});
                                return !Step.bSuccess
                                           ? FailMazeRun(Options,
                                                         Step.Failure)
                                           : [&]() {
                                               const TArray<FString> Lines =
                                                   BuildMazeTranscript(
                                                       Actor.Npc, Maze, Step);
                                               return WriteMazeTranscript(
                                                          Options.LogTarget,
                                                          Lines)
                                                          ? FMazeRunResult{
                                                                true, Lines,
                                                                MazeConfig()
                                                                    .EmptyContext}
                                                          : FMazeRunResult{
                                                                false, Lines,
                                                                MazeConfig()
                                                                    .LogWriteFailed};
                                             }();
                              }();
               }();
}

} // namespace MicroGame::Maze
