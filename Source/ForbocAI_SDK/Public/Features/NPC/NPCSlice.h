#pragma once
/**
 * SYSTEM_OVERRIDE denied unless npc state is coherent end to end
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCAdapters.h"
#include "Features/NPC/NPCTypes.h"

namespace NPCSlice {

using namespace rtk;
using namespace func;

/**
 * Builds the NPC slice reducer and extra cases.
 * User Story: As NPC runtime setup, I need one slice factory so NPC lifecycle
 * actions update state through a single reducer contract.
 * @fn inline Slice<FNPCSliceState> createNPCSlice()
 */
inline Slice<FNPCSliceState> createNPCSlice() {
  return createSlice<FNPCSliceState>(
      TEXT("npc"), FNPCSliceState(),
      [](ActionReducerMapBuilder<FNPCSliceState> &Builder) {
        Builder.addCase(
            NPCActions::setNPCInfoActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetNPCInfoPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              FNPCInternalState NewNPC = Action.PayloadValue.Info;
              NewNPC.StateLog.Empty();
              NewNPC.StateLog.Add(NPCAdapters::makeStateLogEntry(
                  NewNPC.State, NewNPC.State, Action.PayloadValue.Timestamp));
              Next.Entities = NPCAdapters::npcAdapter().upsertOne(Next.Entities, NewNPC);
              Next.ActiveNpcId = NewNPC.Id;
              return Next;
            });
        Builder.addCase(NPCActions::setActiveNPCActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          Next.ActiveNpcId = Action.PayloadValue;
                          return Next;
                        });
        Builder.addCase(
            NPCActions::setNPCStateActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetNPCStatePayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FSetNPCStatePayload &Payload = Action.PayloadValue;
              Next.Entities = NPCAdapters::npcAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.State = Payload.State;
                    Updated.StateLog.Add(NPCAdapters::makeStateLogEntry(
                        Payload.State, Payload.State, Payload.Timestamp));
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            NPCActions::updateNPCStateActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FUpdateNPCStatePayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FUpdateNPCStatePayload &Payload = Action.PayloadValue;
              Next.Entities = NPCAdapters::npcAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.State =
                        NPCAdapters::updateNPCStateLocally(Existing.State, Payload.Delta);
                    Updated.StateLog.Add(NPCAdapters::makeStateLogEntry(
                        Payload.Delta, Updated.State, Payload.Timestamp));
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            NPCActions::addToHistoryActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FAddToHistoryPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FAddToHistoryPayload &Payload = Action.PayloadValue;
              Next.Entities = NPCAdapters::npcAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    FNPCHistoryEntry Entry;
                    Entry.Role = Payload.Role;
                    Entry.Content = Payload.Content;
                    Updated.History.Add(Entry);
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            NPCActions::setHistoryActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetHistoryPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FSetHistoryPayload &Payload = Action.PayloadValue;
              Next.Entities = NPCAdapters::npcAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.History = Payload.History;
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            NPCActions::blockActionActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FBlockActionPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FBlockActionPayload &Payload = Action.PayloadValue;
              Next.Entities = NPCAdapters::npcAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.bIsBlocked = true;
                    Updated.BlockReason = Payload.Reason;
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(NPCActions::clearBlockActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          Next.Entities = NPCAdapters::npcAdapter().updateOne(
                              Next.Entities, Action.PayloadValue,
                              [](const FNPCInternalState &Existing) {
                                FNPCInternalState Updated = Existing;
                                Updated.bIsBlocked = false;
                                Updated.BlockReason.Empty();
                                return Updated;
                              });
                          return Next;
                        });
        Builder.addCase(NPCActions::removeNPCActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          return (Next.Entities = NPCAdapters::npcAdapter().removeOne(
                                      Next.Entities, Action.PayloadValue),
                                  Next.ActiveNpcId == Action.PayloadValue
                                      ? (Next.ActiveNpcId.Empty(), void())
                                      : void(),
                                  Next);
                        });
      });
}

} // namespace NPCSlice
