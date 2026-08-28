#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Run/TwoNpcChatRunTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Persona/PersonaThunks.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Run/Turn/TwoNpcChatRunTurnThunks.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As the micro-game, I need a shared-scene two-NPC chat composed entirely from granular SDK CLI operations with API thought evidence retained in its result. @fn inline FChatRunResult RunTwoNpcChat(FMicroGameStore &Store) */
inline FChatRunResult RunTwoNpcChat(FMicroGameStore &Store) {
  const FTwoNpcChatConfig &Config = TwoNpcChatConfig();
  const FChatCommandResult SceneResult = RunChatCommand(
      Store, Config,
      FormatGenerate(Config, Config.SceneAttribute, Config.EmptyContext));
  return !SceneResult.bSuccess
             ? FChatRunResult{false, {}, SceneResult.Failure}
             : [&]() {
                 const FString Scene = ReadGeneratedValue(
                     Config, SceneResult.Output.Output);
                 const FChatNpcResult NpcA = BuildChatNpc(
                     Store, Config, Scene, Config.EmptyContext);
                 return !NpcA.bSuccess
                            ? FChatRunResult{false, {}, NpcA.Failure}
                            : [&]() {
                                const FChatNpcResult NpcB =
                                    BuildChatNpc(
                                        Store, Config, Scene,
                                        NpcA.Npc.DisplayName);
                                return !NpcB.bSuccess
                                           ? FChatRunResult{
                                                 false, {},
                                                 NpcB.Failure}
                                           : [&]() {
                                               const FChatTurnsResult Turns =
                                                   RunChatTurns(
                                                       Store, Config,
                                                       NpcA.Npc,
                                                       NpcB.Npc,
                                                       NpcA.Npc.Seed,
                                                       Config.Numbers
                                                           .InitialIndex,
                                                       {});
                                               return !Turns.bSuccess
                                                          ? FChatRunResult{
                                                                false,
                                                                Turns
                                                                    .Lines,
                                                                Turns
                                                                    .Failure}
                                                          : [&]() {
                                                              TArray<FString>
                                                                  Lines = {
                                                                      Config
                                                                          .Header};
                                                              Lines.Append(
                                                                  Turns
                                                                      .Lines);
                                                              Lines.Add(
                                                                  Config
                                                                      .Footer);
                                                              return FChatRunResult{
                                                                  true,
                                                                  MoveTemp(
                                                                      Lines),
                                                                  Config
                                                                      .EmptyContext};
                                                            }();
                                             }();
                              }();
               }();
}

} // namespace MicroGame::TwoNpcChat
