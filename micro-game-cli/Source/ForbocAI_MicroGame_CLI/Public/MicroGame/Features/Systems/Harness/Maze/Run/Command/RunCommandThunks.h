#pragma once

#include "MicroGame/Features/Components/Harness/Maze/Run/MazeRunTypes.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Formatting/FormattingAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/VerificationCommandThunks.h"

namespace MicroGame::Maze {

/**
 * User Story: As shared Maze orchestration, I need every granular operation executed through the real SDK CLI boundary and returned as explicit success or failure.
 * @fn inline FMazeCommandResult RunMazeCommand( FMicroGameStore &Store, const FMazeRunOptions &Options, const FString &Command)
 */
inline FMazeCommandResult RunMazeCommand(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FString &Command) {
  FCommandSpec Spec;
  Spec.Group = MazeConfig().Group;
  Spec.Command = Command;
  const CommandRunner::FCommandOutput Output =
      VerificationThunksDetail::ExecuteCommand(Spec, Store);
  const bool bSuccess =
      Output.Status ==
      VerificationVocabularyAdapters::GameRuntimeData().statuses.ok;
  return {bSuccess,
          Output,
          bSuccess
              ? MazeConfig().EmptyContext
              : ApplyMazeToken(Options.CommandFailureFormat,
                               MazeConfig().Tokens.Output, Output.Output)};
}

} // namespace MicroGame::Maze
