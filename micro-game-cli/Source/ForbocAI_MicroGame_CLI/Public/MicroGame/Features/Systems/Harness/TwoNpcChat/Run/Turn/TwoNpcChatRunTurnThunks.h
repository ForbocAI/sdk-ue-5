#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Run/TwoNpcChatRunTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Command/TwoNpcChatCommandThunks.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Response/TwoNpcChatResponseAdapters.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Transcript/TranscriptAdapters.h"

namespace MicroGame::TwoNpcChat {

/**
 * User Story: As a two-NPC conversation, I need each speaker recalled, decided by the API, persisted, and alternated while thought evidence remains attached to the same turn.
 * @fn inline FChatTurnsResult RunChatTurns( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FChatNpc &Speaker, const FChatNpc &Listener, const FString &InputLine, int32 Turn, TArray<FString> Lines)
 */
inline FChatTurnsResult RunChatTurns(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FChatNpc &Speaker, const FChatNpc &Listener,
    const FString &InputLine, int32 Turn, TArray<FString> Lines) {
  return Turn >= Config.TurnCount
             ? FChatTurnsResult{true, Lines, Config.EmptyContext}
             : [&]() {
                 const FChatCommandResult Recalled = RunChatCommand(
                     Store, Config, FormatRecall(Config, Speaker.Id));
                 return !Recalled.bSuccess
                            ? FChatTurnsResult{false, Lines,
                                               Recalled.Failure}
                            : [&]() {
                                const FChatCommandResult Decided =
                                    RunChatCommand(
                                        Store, Config,
                                        FormatDecide(Config, Speaker.Id,
                                                     InputLine));
                                return !Decided.bSuccess
                                           ? FChatTurnsResult{
                                                 false, Lines,
                                                 Decided.Failure}
                                           : [&]() {
                                               const FString Dialogue =
                                                   ReadDialogue(
                                                       Config,
                                                       Decided.Output
                                                           .Output);
                                               const FString Thought =
                                                   ReadThoughtResult(
                                                       Config,
                                                       Decided.Output
                                                           .Output);
                                               const FChatCommandResult
                                                   Stored =
                                                       RunChatCommand(
                                                           Store, Config,
                                                           FormatMemoryStore(
                                                               Config,
                                                               Speaker.Id,
                                                               Dialogue));
                                               return !Stored.bSuccess
                                                          ? FChatTurnsResult{
                                                                false,
                                                                Lines,
                                                                Stored
                                                                    .Failure}
                                                          : (Lines.Add(
                                                                 FormatLine(
                                                                     Config,
                                                                     Turn +
                                                                         Config
                                                                             .Numbers
                                                                             .NextIndex,
                                                                     Speaker
                                                                         .DisplayName,
                                                                     Dialogue)),
                                                             Lines.Add(
                                                                 FormatThought(
                                                                     Config,
                                                                     Speaker
                                                                         .DisplayName,
                                                                     Thought)),
                                                             Lines.Add(
                                                                 Config
                                                                     .BlankLine),
                                                             RunChatTurns(
                                                                 Store,
                                                                 Config,
                                                                 Listener,
                                                                 Speaker,
                                                                 Dialogue,
                                                                 Turn +
                                                                     Config
                                                                         .Numbers
                                                                         .NextIndex,
                                                                 MoveTemp(
                                                                     Lines)));
                                             }();
                              }();
               }();
}

} // namespace MicroGame::TwoNpcChat
