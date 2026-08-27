#pragma once

#include "CoreMinimal.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/TwoNpcChatAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/CommandThunks.h"
// The two-NPC chat is the GAME's own orchestration (it lives in the game, not
// the SDK core). It composes the granular CLI commands, each of which executes
// an SDK core primitive: npc generate x N -> npc create -> memory store x N,
// then per turn npc decide -> memory store. Any SDK consumer (Platform,
// demo-ue) can do the same by composing those core primitives directly.

namespace MicroGame {

namespace TwoNpcChat {

/**
 * User Story: As the two-NPC chat game orchestrator, I need each generated command run through the real forbocai CLI so game state reflects the exercised SDK primitive.
 * @fn inline CommandRunner::FCommandOutput RunChatCommand( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Command)
 */
inline CommandRunner::FCommandOutput RunChatCommand(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FString &Command) {
  FCommandSpec Spec;
  Spec.Group = Config.Group;
  Spec.Command = Command;
  return VerificationThunksDetail::ExecuteCommand(Spec, Store);
}

/** User Story: As the two-NPC chat game orchestrator, I need each persona attribute SLM-generated in order and threaded as context so the persona stays consistent. @fn inline TArray<FString> GenerateAttributes(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, int32 Index, const FString &Context, TArray<FString> Values) */
inline TArray<FString> GenerateAttributes(FMicroGameStore &Store,
                                          const FTwoNpcChatConfig &Config,
                                          int32 Index, const FString &Context,
                                          TArray<FString> Values) {
  return Index >= Config.PersonaAttributes.Num()
             ? Values
             : [&]() {
                 const FString Value = ReadGeneratedValue(
                     Config, RunChatCommand(Store, Config,
                                            FormatGenerate(Config,
                                                           Config.PersonaAttributes[Index],
                                                           Context))
                                 .Output);
                 const FString Next = ExtendContext(Config, Context, Value);
                 Values.Add(Value);
                 return GenerateAttributes(
                     Store, Config, Index + Config.Numbers.NextIndex, Next,
                     Values);
               }();
}

/** User Story: As the two-NPC chat game orchestrator, I need each generated attribute persisted to the NPC's vector db through a composed memory store command so a later session can recall the same NPC. @fn inline void PersistAttributes(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Id, const TArray<FString> &Memories, int32 Index) */
inline void PersistAttributes(FMicroGameStore &Store,
                              const FTwoNpcChatConfig &Config, const FString &Id,
                              const TArray<FString> &Memories, int32 Index) {
  Index >= Memories.Num()
      ? void()
      : (RunChatCommand(Store, Config,
                        FormatMemoryStore(Config, Id, Memories[Index])),
         PersistAttributes(Store, Config, Id, Memories,
                           Index + Config.Numbers.NextIndex));
}

/** User Story: As the two-NPC chat game orchestrator, I need one participant SLM-generated, created Redux-only, then its persona persisted to the vector db via composed memory store commands. @fn inline FChatNpc BuildNpc(FMicroGameStore &Store, const FTwoNpcChatConfig &Config) */
inline FChatNpc BuildNpc(FMicroGameStore &Store,
                         const FTwoNpcChatConfig &Config) {
  const FString Name = ReadGeneratedValue(
      Config, RunChatCommand(Store, Config,
                             FormatGenerate(Config, Config.NameAttribute,
                                            Config.EmptyContext))
                  .Output);
  const TArray<FString> Values = GenerateAttributes(
      Store, Config, Config.Numbers.InitialIndex, Config.EmptyContext,
      TArray<FString>());
  const FString Persona = JoinPersona(Config, Values);
  const FString Id = ReadNpcId(
      Config, RunChatCommand(Store, Config, FormatCreate(Config, Persona)).Output);
  TArray<FString> Memories;
  Memories.Add(Name);
  Memories.Append(Values);
  PersistAttributes(Store, Config, Id, Memories,
                    Config.Numbers.InitialIndex);
  return FChatNpc{Id, Name, SeedValue(Config, Values)};
}

/** User Story: As the two-NPC chat game orchestrator, I need each NPC to react to the other's last line via the decide-only command, then persist that line via a composed memory store, alternating for the authored turn count. @fn inline TArray<FString> RunTurns(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FChatNpc &Speaker, const FChatNpc &Listener, const FString &InputLine, int32 Turn, TArray<FString> Lines) */
inline TArray<FString> RunTurns(FMicroGameStore &Store,
                                const FTwoNpcChatConfig &Config,
                                const FChatNpc &Speaker, const FChatNpc &Listener,
                                const FString &InputLine, int32 Turn,
                                TArray<FString> Lines) {
  return Turn >= Config.TurnCount
             ? Lines
             : [&]() {
                 const FString Dialogue = ReadDialogue(
                     Config, RunChatCommand(Store, Config,
                                            FormatDecide(Config, Speaker.Id,
                                                         InputLine))
                                 .Output);
                 const FString Line = FormatLine(Config, Speaker.Name, Dialogue);
                 RunChatCommand(Store, Config,
                                FormatMemoryStore(Config, Speaker.Id, Line));
                 Lines.Add(Line);
                 return RunTurns(Store, Config, Listener, Speaker, Dialogue,
                                 Turn + Config.Numbers.NextIndex, Lines);
               }();
}

/** User Story: As the micro-game, I need a client-orchestrated two-NPC chat that SLM-generates both personas, opens from a generated seed, alternates real decide turns, and persists every line to the vector db. @fn inline TArray<FString> RunTwoNpcChat(FMicroGameStore &Store) */
inline TArray<FString> RunTwoNpcChat(FMicroGameStore &Store) {
  const FTwoNpcChatConfig &Config = TwoNpcChatConfig();
  const FChatNpc NpcA = BuildNpc(Store, Config);
  const FChatNpc NpcB = BuildNpc(Store, Config);
  TArray<FString> Lines;
  Lines.Add(Config.Header);
  Lines.Append(
      RunTurns(Store, Config, NpcA, NpcB, NpcA.Seed,
               Config.Numbers.InitialIndex, TArray<FString>()));
  Lines.Add(Config.Footer);
  return Lines;
}

} // namespace TwoNpcChat

} // namespace MicroGame
