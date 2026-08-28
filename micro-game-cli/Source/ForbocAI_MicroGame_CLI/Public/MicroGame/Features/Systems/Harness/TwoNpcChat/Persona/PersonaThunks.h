#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Memory/TwoNpcChatMemoryTypes.h"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/Run/TwoNpcChatRunTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Command/TwoNpcChatCommandThunks.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Persona/PersonaAdapters.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Response/TwoNpcChatResponseAdapters.h"

namespace MicroGame::TwoNpcChat {

/**
 * User Story: As two-NPC persona generation, I need attributes generated recursively with prior values threaded through authored context.
 * @fn inline FChatValuesResult GenerateChatAttributes( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, int32 Index, const FString &Context, TArray<FString> Values)
 */
inline FChatValuesResult GenerateChatAttributes(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config, int32 Index,
    const FString &Context, TArray<FString> Values) {
  return Index >= Config.PersonaAttributes.Num()
             ? FChatValuesResult{true, Values, Config.EmptyContext}
             : [&]() {
                 const FChatCommandResult Generated = RunChatCommand(
                     Store, Config,
                     FormatGenerate(Config, Config.PersonaAttributes[Index],
                                    Context));
                 const FString Value =
                     Generated.bSuccess
                         ? ReadGeneratedValue(Config, Generated.Output.Output)
                         : Config.EmptyContext;
                 return !Generated.bSuccess
                            ? FChatValuesResult{false, Values,
                                                Generated.Failure}
                            : (Values.Add(Value),
                               GenerateChatAttributes(
                                   Store, Config,
                                   Index + Config.Numbers.NextIndex,
                                   ExtendContext(Config, Context, Value),
                                   MoveTemp(Values)));
               }();
}

/**
 * User Story: As two-NPC vector persistence, I need typed memories stored recursively through granular SDK CLI commands with the first failure retained.
 * @fn inline FChatOperationResult PersistChatMemories( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Id, const TArray<FTypedMemory> &Memories, int32 Index)
 */
inline FChatOperationResult PersistChatMemories(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FString &Id, const TArray<FTypedMemory> &Memories, int32 Index) {
  return Index >= Memories.Num()
             ? FChatOperationResult{true, Config.EmptyContext}
             : [&]() {
                 const FChatCommandResult Stored = RunChatCommand(
                     Store, Config,
                     FormatTypedMemoryStore(
                         Config, Id, Memories[Index].Text,
                         Memories[Index].Type));
                 return Stored.bSuccess
                            ? PersistChatMemories(
                                  Store, Config, Id, Memories,
                                  Index + Config.Numbers.NextIndex)
                            : FChatOperationResult{false, Stored.Failure};
               }();
}

/**
 * User Story: As two-NPC setup, I need one generated participant created, identity-grounded, and persisted with structured memories through granular SDK CLI operations.
 * @fn inline FChatNpcResult BuildChatNpc( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Scene, const FString &Peer)
 */
inline FChatNpcResult BuildChatNpc(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FString &Scene, const FString &Peer) {
  const FChatCommandResult NameResult = RunChatCommand(
      Store, Config, FormatGenerate(Config, Config.NameAttribute, Scene));
  return !NameResult.bSuccess
             ? FChatNpcResult{false, {}, NameResult.Failure}
             : [&]() {
                 const FString Name =
                     ReadGeneratedValue(Config, NameResult.Output.Output);
                 const FChatValuesResult Attributes =
                     GenerateChatAttributes(
                         Store, Config, Config.Numbers.InitialIndex, Scene, {});
                 return !Attributes.bSuccess
                            ? FChatNpcResult{false, {}, Attributes.Failure}
                            : [&]() {
                                const FChatCommandResult Created =
                                    RunChatCommand(
                                        Store, Config,
                                        FormatCreate(
                                            Config,
                                            JoinPersona(
                                                Config,
                                                Attributes.Values)));
                                const FString Id =
                                    Created.bSuccess
                                        ? ReadNpcId(
                                              Config,
                                              Created.Output.Output)
                                        : Config.EmptyContext;
                                return !Created.bSuccess
                                           ? FChatNpcResult{
                                                 false, {},
                                                 Created.Failure}
                                           : [&]() {
                                               const FString Role =
                                                   RoleValue(
                                                       Config,
                                                       Attributes.Values);
                                               const FChatCommandResult
                                                   Updated =
                                                       RunChatCommand(
                                                           Store, Config,
                                                           FormatIdentityUpdate(
                                                               Config, Id,
                                                               Name,
                                                               Role));
                                               const FChatOperationResult
                                                   Persisted =
                                                       Updated.bSuccess
                                                           ? PersistChatMemories(
                                                                 Store,
                                                                 Config,
                                                                 Id,
                                                                 PersonaMemories(
                                                                     Config,
                                                                     Scene,
                                                                     Attributes
                                                                         .Values,
                                                                     Peer),
                                                                 Config
                                                                     .Numbers
                                                                     .InitialIndex)
                                                           : FChatOperationResult{
                                                                 false,
                                                                 Updated
                                                                     .Failure};
                                               return Persisted.bSuccess
                                                          ? FChatNpcResult{
                                                                true,
                                                                {Id,
                                                                 Name,
                                                                 FormatDisplayName(
                                                                     Config,
                                                                     Name,
                                                                     Role),
                                                                 SeedValue(
                                                                     Config,
                                                                     Attributes
                                                                         .Values)},
                                                                Config
                                                                    .EmptyContext}
                                                          : FChatNpcResult{
                                                                false, {},
                                                                Persisted
                                                                    .Failure};
                                             }();
                              }();
               }();
}

} // namespace MicroGame::TwoNpcChat
