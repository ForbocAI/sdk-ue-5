#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Command/MazeCommandAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Response/MazeResponseAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Run/Command/RunCommandThunks.h"
#include "MicroGame/Features/Systems/Harness/Maze/Topology/Decision/DecisionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Transcript/Turn/TurnAdapters.h"

namespace MicroGame::Maze {

/**
 * User Story: As a fresh Maze actor, I need one unscored recall and decision round trip before benchmark turns begin.
 * @fn inline FMazeCommandResult WarmMazeNpc( FMicroGameStore &Store, const FMazeRunOptions &Options, const FMazeCommands &Commands, const FMazeNpc &Npc, const FMaze &Maze)
 */
inline FMazeCommandResult WarmMazeNpc(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FMazeCommands &Commands, const FMazeNpc &Npc,
    const FMaze &Maze) {
  const TArray<FString> Open = OpenDirections(Maze, Maze.Start);
  const FString Prompt =
      FormatTurnPrompt(Maze.Start, Open, MazeConfig().Numbers.Zero);
  const FMazeCommandResult Recall = RunMazeCommand(
      Store, Options, FormatMazeRecall(Commands, Npc.Id));
  return !Recall.bSuccess
             ? Recall
             : RunMazeCommand(
                   Store, Options,
                   FormatMazeDecide(Commands, Npc.Id, Prompt, Open, {}, {}));
}

/**
 * User Story: As RAG-grounded Maze turns, I need the query, reply, and movement outcome persisted in order through granular memory commands.
 * @fn inline FMazeCommandResult PersistMazeTurn( FMicroGameStore &Store, const FMazeRunOptions &Options, const FMazeCommands &Commands, const FMazeNpc &Npc, const FString &Prompt, const FString &Dialogue, const FString &Outcome)
 */
inline FMazeCommandResult PersistMazeTurn(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FMazeCommands &Commands, const FMazeNpc &Npc,
    const FString &Prompt, const FString &Dialogue,
    const FString &Outcome) {
  const FMazeCommandResult Query = RunMazeCommand(
      Store, Options,
      FormatMazeMemoryStore(Commands, Npc.Id, Prompt));
  return !Query.bSuccess
             ? Query
             : [&]() {
                 const FMazeCommandResult Reply = RunMazeCommand(
                     Store, Options,
                     FormatMazeMemoryStore(Commands, Npc.Id, Dialogue));
                 return !Reply.bSuccess
                            ? Reply
                            : RunMazeCommand(
                                  Store, Options,
                                  FormatMazeTypedMemoryStore(
                                      Commands, Npc.Id, Outcome,
                                      MazeConfig().GoalMemoryType));
               }();
}

/**
 * User Story: As Maze fail-fast orchestration, I need the original benchmark counters and transcript retained beside a command failure.
 * @fn inline FMazeStep FailedMazeStep( const FString &Failure, const FString &Cell, int32 DeadEnds, int32 Turn, int32 RealChoices, int32 OptimalChoices, const TArray<FString> &Lines)
 */
inline FMazeStep FailedMazeStep(
    const FString &Failure, const FString &Cell, int32 DeadEnds,
    int32 Turn, int32 RealChoices, int32 OptimalChoices,
    const TArray<FString> &Lines) {
  return {false, Failure, Cell, DeadEnds, false,
          Turn - MazeConfig().Numbers.One, RealChoices, OptimalChoices,
          Lines};
}

/**
 * User Story: As the Maze benchmark, I need each grounded recall and decision recursively measured, persisted, and advanced until solved or exhausted.
 * @fn inline FMazeStep RunMazeTurns( FMicroGameStore &Store, const FMazeRunOptions &Options, const FMazeCommands &Commands, const FMazeNpc &Npc, const FMaze &Maze, const FString &Cell, int32 Turn, int32 DeadEnds, int32 RealChoices, int32 OptimalChoices, TArray<FString> Lines, TArray<FString> Visited)
 */
inline FMazeStep RunMazeTurns(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FMazeCommands &Commands, const FMazeNpc &Npc,
    const FMaze &Maze, const FString &Cell, int32 Turn,
    int32 DeadEnds, int32 RealChoices, int32 OptimalChoices,
    TArray<FString> Lines, TArray<FString> Visited) {
  return Cell == Maze.Exit || Turn > MazeConfig().TurnLimit
             ? FMazeStep{true,
                         MazeConfig().EmptyContext,
                         Cell,
                         DeadEnds,
                         Cell == Maze.Exit,
                         Turn - MazeConfig().Numbers.One,
                         RealChoices,
                         OptimalChoices,
                         Lines}
             : [&]() {
                 const FWardOutcome Ward =
                     MeasureWardOutcome(Maze, Cell, Visited);
                 const FString Prompt = FormatTurnPrompt(
                     Cell, Ward.Options, Turn - MazeConfig().Numbers.One);
                 const FMazeCommandResult Recall = RunMazeCommand(
                     Store, Options, FormatMazeRecall(Commands, Npc.Id));
                 return !Recall.bSuccess
                            ? FailedMazeStep(
                                  Recall.Failure, Cell, DeadEnds, Turn,
                                  RealChoices, OptimalChoices, Lines)
                            : [&]() {
                                const FMazeCommandResult Decision =
                                    RunMazeCommand(
                                        Store, Options,
                                        FormatMazeDecide(
                                            Commands, Npc.Id, Prompt,
                                            Ward.Options,
                                            Ward.VisitedDirections,
                                            Ward.AvoidDirections));
                                return !Decision.bSuccess
                                           ? FailedMazeStep(
                                                 Decision.Failure, Cell,
                                                 DeadEnds, Turn,
                                                 RealChoices, OptimalChoices,
                                                 Lines)
                                           : [&]() {
                                               const FString Dialogue =
                                                   ReadMazeDialogue(
                                                       Decision.Output.Output);
                                               const func::Maybe<FString>
                                                   Direction = ParseDirection(
                                                       Dialogue, Ward.Options);
                                               const FString Next =
                                                   Direction.hasValue
                                                       ? Maze.Cells
                                                             .FindChecked(Cell)
                                                             .FindChecked(
                                                                 Direction.value)
                                                       : Cell;
                                               const bool bDeadEnd =
                                                   Direction.hasValue &&
                                                   Maze.DeadEnds.Contains(Next);
                                               const bool bOptimal =
                                                   Ward.bRealChoice &&
                                                   Direction.hasValue &&
                                                   Ward.OptimalDirections.Contains(
                                                       Direction.value);
                                               const FString Outcome =
                                                   Direction.hasValue
                                                       ? FormatMazeMoved(
                                                             Turn, Cell,
                                                             Direction.value,
                                                             Next, bDeadEnd)
                                                       : FormatMazeUnparsed(
                                                             Turn, Cell,
                                                             Dialogue);
                                               Options.bDebug
                                                   ? (Lines.Add(FormatMazeWard(
                                                          Turn, Cell, Ward)),
                                                      Lines.Add(FormatMazeQuery(
                                                          Turn, Cell, Prompt)),
                                                      void())
                                                   : void();
                                               Lines.Add(FormatMazeReason(
                                                   Turn, Cell,
                                                   ReadMazeReason(
                                                       Decision.Output.Output)));
                                               Lines.Add(FormatMazeThought(
                                                   Turn, Cell,
                                                   ReadMazeThought(
                                                       Decision.Output.Output)));
                                               Lines.Add(Outcome);
                                               const FMazeCommandResult Saved =
                                                   PersistMazeTurn(
                                                       Store, Options, Commands,
                                                       Npc, Prompt, Dialogue,
                                                       Outcome);
                                               return !Saved.bSuccess
                                                          ? FailedMazeStep(
                                                                Saved.Failure,
                                                                Cell, DeadEnds,
                                                                Turn,
                                                                RealChoices,
                                                                OptimalChoices,
                                                                Lines)
                                                          : (Visited.Add(Next),
                                                             RunMazeTurns(
                                                                 Store, Options,
                                                                 Commands, Npc,
                                                                 Maze, Next,
                                                                 Turn + MazeConfig()
                                                                            .Numbers
                                                                            .One,
                                                                 DeadEnds +
                                                                     (bDeadEnd
                                                                          ? MazeConfig()
                                                                                .Numbers
                                                                                .One
                                                                          : MazeConfig()
                                                                                .Numbers
                                                                                .Zero),
                                                                 RealChoices +
                                                                     (Ward.bRealChoice
                                                                          ? MazeConfig()
                                                                                .Numbers
                                                                                .One
                                                                          : MazeConfig()
                                                                                .Numbers
                                                                                .Zero),
                                                                 OptimalChoices +
                                                                     (bOptimal
                                                                          ? MazeConfig()
                                                                                .Numbers
                                                                                .One
                                                                          : MazeConfig()
                                                                                .Numbers
                                                                                .Zero),
                                                                 MoveTemp(Lines),
                                                                 MoveTemp(Visited)));
                                             }();
                              }();
               }();
}

} // namespace MicroGame::Maze
