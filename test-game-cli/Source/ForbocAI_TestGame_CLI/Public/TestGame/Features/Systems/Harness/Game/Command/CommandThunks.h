#pragma once

#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame::GameThunksDetail {

/** User Story: As a command coverage runner, I need to reduce real CLI results through a stable signature so game state reflects each exercised SDK command. @fn inline void ApplyCommandResult( const FCommandSpec &Command, const CommandRunner::FCommandOutput &CommandResult, FTestGameStore &Store) */
inline void ApplyCommandResult(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &CommandResult,
    FTestGameStore &Store) {
  Command.Group == ECommandGroup::NpcProcessChat
      ? [&]() {
          const FParsedVerdict Verdict =
              GameAdapters::ParseVerdict(CommandResult.Output);
          const FString NpcId = GameAdapters::ExtractNpcId(Command.Command);
          (Verdict.bValid && !NpcId.IsEmpty())
              ? [&]() {
                  NPCsActions::FApplyNpcVerdictPayload Payload;
                  Payload.Id = NpcId;
                  Payload.Action.Type = Verdict.ActionType;
                  Payload.Action.TargetHex = Verdict.TargetHex;
                  Payload.Action.bHasTargetHex = true;
                  Payload.StateDelta.Suspicion = Verdict.SuspicionDelta;
                  Payload.StateDelta.bHasSuspicion =
                      Verdict.SuspicionDelta != 0;
                  Store.dispatch(NPCsActions::ApplyNpcVerdict(Payload));
                }()
              : void();
        }()
      : void();

  (Command.Group == ECommandGroup::BridgeValidate &&
   CommandResult.Status == ETranscriptStatus::Error)
      ? (Store.dispatch(UIActions::addMessage(
             FString(TEXT("Bridge validation failed: ")) +
             CommandResult.Output)),
         void())
      : void();
}

} // namespace TestGame::GameThunksDetail
