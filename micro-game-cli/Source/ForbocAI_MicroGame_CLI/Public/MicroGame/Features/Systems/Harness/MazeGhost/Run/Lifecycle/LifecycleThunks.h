#pragma once

#include "MicroGame/Features/Components/Harness/Maze/Run/MazeRunTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"
#include "MicroGame/Features/Systems/Harness/MazeGhost/Definition/MazeGhostDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/VerificationCommandThunks.h"

namespace MicroGame::MazeGhost {

/**
 * User Story: As Maze Ghost lifecycle composition, I need every session operation executed only through its Ghost CLI command and returned as explicit success or failure.
 * @fn inline Maze::FMazeCommandResult RunGhostLifecycleCommand( FMicroGameStore &Store, const FString &Command)
 */
inline Maze::FMazeCommandResult RunGhostLifecycleCommand(
    FMicroGameStore &Store, const FString &Command) {
  FCommandSpec Spec;
  Spec.Group = MazeGhostConfig().Group;
  Spec.Command = Command;
  const CommandRunner::FCommandOutput Output =
      VerificationThunksDetail::ExecuteCommand(Spec, Store);
  const bool bSuccess =
      Output.Status ==
      VerificationVocabularyAdapters::GameRuntimeData().statuses.ok;
  return {bSuccess,
          Output,
          bSuccess
              ? Maze::MazeConfig().EmptyContext
              : Maze::ApplyMazeToken(MazeGhostConfig().CommandFailed,
                                     MazeGhostConfig().OutputToken,
                                     Output.Output)};
}

/**
 * User Story: As durable Ghost sessions, I need a failed Maze run to attempt the authored stop command while preserving the original failure.
 * @fn inline Maze::FMazeRunResult StopGhostAfterFailure( FMicroGameStore &Store, const Maze::FMazeRunResult &Failure)
 */
inline Maze::FMazeRunResult StopGhostAfterFailure(
    FMicroGameStore &Store, const Maze::FMazeRunResult &Failure) {
  RunGhostLifecycleCommand(Store, MazeGhostConfig().StopCommand);
  return Failure;
}

} // namespace MicroGame::MazeGhost
