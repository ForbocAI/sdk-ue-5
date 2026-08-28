#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Run/TwoNpcChatRunTypes.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Definition/TwoNpcChatDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Invocation/InvocationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/VerificationCommandThunks.h"

namespace MicroGame::TwoNpcChat {

/**
 * User Story: As two-NPC orchestration, I need every granular operation executed through the real SDK CLI boundary and returned as explicit success or failure.
 * @fn inline FChatCommandResult RunChatCommand( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Command)
 */
inline FChatCommandResult RunChatCommand(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FString &Command) {
  FCommandSpec Spec;
  Spec.Group = Config.Group;
  Spec.Command = Command;
  const CommandRunner::FCommandOutput Output =
      VerificationThunksDetail::ExecuteCommand(Spec, Store);
  const bool bSuccess =
      Output.Status ==
      VerificationVocabularyAdapters::GameRuntimeData().statuses.ok;
  return {
      bSuccess, Output,
      bSuccess ? Config.EmptyContext
               : ApplyToken(Config.CommandFailed, Config.Tokens.Output,
                            Output.Output)};
}

} // namespace MicroGame::TwoNpcChat
