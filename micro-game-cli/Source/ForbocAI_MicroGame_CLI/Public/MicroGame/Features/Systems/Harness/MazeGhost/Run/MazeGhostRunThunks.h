#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Run/MazeRunThunks.h"
#include "MicroGame/Features/Systems/Harness/MazeGhost/Run/Lifecycle/LifecycleThunks.h"

namespace MicroGame::MazeGhost {

/**
 * User Story: As Ghost level analysis, I need API-owned results appended after the unchanged shared Maze transcript and persisted to the Ghost evidence target.
 * @fn inline Maze::FMazeRunResult CompleteMazeGhostRun( const Maze::FMazeRunOptions &Options, const Maze::FMazeRunResult &MazeResult, const FString &Analysis)
 */
inline Maze::FMazeRunResult CompleteMazeGhostRun(
    const Maze::FMazeRunOptions &Options,
    const Maze::FMazeRunResult &MazeResult, const FString &Analysis) {
  const TArray<FString> Lines = func::concat_arrays<FString>(
      {MazeResult.Lines,
       {MazeGhostConfig().AnalysisHeader, Analysis,
        MazeGhostConfig().AnalysisFooter}});
  return Maze::WriteMazeTranscript(Options.LogTarget, Lines)
             ? Maze::FMazeRunResult{true, Lines,
                                    Maze::MazeConfig().EmptyContext}
             : Maze::FMazeRunResult{false, Lines,
                                    Maze::MazeConfig().LogWriteFailed};
}

/**
 * User Story: As Ghost pre-playtesting, I need the ordinary Maze mechanic run unchanged inside a Ghost session with every injected operation using only Ghost CLI commands and endpoints.
 * @fn inline Maze::FMazeRunResult RunMazeGhost( FMicroGameStore &Store, bool bDebug, const func::Maybe<uint32> &Seed)
 */
inline Maze::FMazeRunResult RunMazeGhost(
    FMicroGameStore &Store, bool bDebug,
    const func::Maybe<uint32> &Seed) {
  const FMazeGhostConfig &Data = MazeGhostConfig();
  Maze::FMazeRunOptions Options;
  Options.bDebug = bDebug;
  Options.Seed = Seed;
  Options.LogTarget = {Data.LogFileEnvKey, Data.DefaultLogPath};
  Options.CommandFailureFormat = Data.CommandFailed;
  const Maze::FMazeCommandResult Started =
      RunGhostLifecycleCommand(Store, Data.RunCommand);
  return !Started.bSuccess
             ? Maze::FMazeRunResult{false, {}, Started.Failure}
             : [&]() {
                 const Maze::FMazeRunResult MazeResult =
                     Maze::RunMaze(Store, Data.Commands, Options);
                 return !MazeResult.bSuccess
                            ? StopGhostAfterFailure(Store, MazeResult)
                            : [&]() {
                                const Maze::FMazeCommandResult Stopped =
                                    RunGhostLifecycleCommand(
                                        Store, Data.StopCommand);
                                return !Stopped.bSuccess
                                           ? Maze::FMazeRunResult{
                                                 false, MazeResult.Lines,
                                                 Stopped.Failure}
                                           : [&]() {
                                               const Maze::FMazeCommandResult
                                                   Results =
                                                       RunGhostLifecycleCommand(
                                                           Store,
                                                           Data.ResultsCommand);
                                               return !Results.bSuccess
                                                          ? Maze::FMazeRunResult{
                                                                false,
                                                                MazeResult.Lines,
                                                                Results.Failure}
                                                          : CompleteMazeGhostRun(
                                                                Options,
                                                                MazeResult,
                                                                Results.Output
                                                                    .Output);
                                             }();
                              }();
               }();
}

} // namespace MicroGame::MazeGhost
