#pragma once
/**
 * SYSTEM_OVERRIDE denied unless npc state is coherent end to end
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Features/NPC/NPCSliceActions.h"
#include "NPC/NPCTypes.h"
#include "Types.h"

namespace NPCSlice {

using namespace rtk;
using namespace func;

/**
 * Builds the NPC slice reducer and extra cases.
 * User Story: As NPC runtime setup, I need one slice factory so NPC lifecycle
 * actions update state through a single reducer contract.
 */
inline Slice<FNPCSliceState> createNPCSlice() {
  return createSlice<FNPCSliceState>(
      TEXT("npc"), FNPCSliceState(),
      [](ActionReducerMapBuilder<FNPCSliceState> &Builder) {
        Builder.addCase(
            Actions::setNPCInfoActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FNPCInternalState> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              FNPCInternalState NewNPC = Action.PayloadValue;
              NewNPC.StateLog.Empty();
              NewNPC.StateLog.Add(MakeStateLogEntry(NewNPC.State, NewNPC.State));
              Next.Entities = GetNPCAdapter().upsertOne(Next.Entities, NewNPC);
              Next.ActiveNpcId = NewNPC.Id;
              return Next;
            });
        Builder.addCase(Actions::setActiveNPCActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          Next.ActiveNpcId = Action.PayloadValue;
                          return Next;
                        });
        Builder.addCase(
            Actions::setNPCStateActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetNPCStatePayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FSetNPCStatePayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.State = Payload.State;
                    Updated.StateLog.Add(
                        MakeStateLogEntry(Payload.State, Payload.State));
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            Actions::updateNPCStateActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FUpdateNPCStatePayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FUpdateNPCStatePayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.State =
                        MergeStateDelta(Existing.State, Payload.Delta);
                    Updated.StateLog.Add(
                        MakeStateLogEntry(Payload.Delta, Updated.State));
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            Actions::addToHistoryActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FAddToHistoryPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FAddToHistoryPayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
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
            Actions::setHistoryActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetHistoryPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FSetHistoryPayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.History = Payload.History;
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            Actions::setLastActionActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FSetLastActionPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FSetLastActionPayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    return (Payload.bHasAction
                                ? (Updated.LastAction = Payload.Action,
                                   Updated.bHasLastAction = true, void())
                                : (Updated.LastAction = FAgentAction{},
                                   Updated.bHasLastAction = false, void()),
                            Updated);
                  });
              return Next;
            });
        Builder.addCase(
            Actions::blockActionActionCreator(),
            [](const FNPCSliceState &State,
               const Action<FBlockActionPayload> &Action) -> FNPCSliceState {
              FNPCSliceState Next = State;
              const FBlockActionPayload &Payload = Action.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, Payload.Id,
                  [Payload](const FNPCInternalState &Existing) {
                    FNPCInternalState Updated = Existing;
                    Updated.bIsBlocked = true;
                    Updated.BlockReason = Payload.Reason;
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(Actions::clearBlockActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          Next.Entities = GetNPCAdapter().updateOne(
                              Next.Entities, Action.PayloadValue,
                              [](const FNPCInternalState &Existing) {
                                FNPCInternalState Updated = Existing;
                                Updated.bIsBlocked = false;
                                Updated.BlockReason.Empty();
                                return Updated;
                              });
                          return Next;
                        });
        Builder.addCase(Actions::removeNPCActionCreator(),
                        [](const FNPCSliceState &State,
                           const Action<FString> &Action) -> FNPCSliceState {
                          FNPCSliceState Next = State;
                          return (Next.Entities = GetNPCAdapter().removeOne(
                                      Next.Entities, Action.PayloadValue),
                                  Next.ActiveNpcId == Action.PayloadValue
                                      ? (Next.ActiveNpcId.Empty(), void())
                                      : void(),
                                  Next);
                        });
      });
}

/**
 * Selects a single NPC by id.
 * User Story: As NPC lookups, I need direct access to one NPC so reducers,
 * thunks, and UI code can target the correct entity.
 */
inline func::Maybe<FNPCInternalState> selectNPCById(const FNPCSliceState &State,
                                                    const FString &Id) {
  return GetNPCAdapter().getSelectors().selectById(State.Entities, Id);
}

/**
 * Selects all NPC ids currently held in adapter order.
 * User Story: As NPC list consumers, I need entity ids without materializing
 * every NPC so callers can mirror RTK entity selector behavior.
 */
inline TArray<FString> selectNPCIds(const FNPCSliceState &State) {
  return GetNPCAdapter().getSelectors().selectIds(State.Entities);
}

/**
 * Selects the NPC entity map keyed by id.
 * User Story: As direct entity consumers, I need the id-to-NPC map so UE
 * callers can mirror TS adapter selector behavior.
 */
inline TMap<FString, FNPCInternalState>
selectNPCEntities(const FNPCSliceState &State) {
  return State.Entities.entities;
}

/**
 * Selects every NPC currently held in slice state.
 * User Story: As NPC inspection flows, I need the full entity list so tools
 * and runtime systems can review current NPC state.
 */
inline TArray<FNPCInternalState> selectAllNPCs(const FNPCSliceState &State) {
  return GetNPCAdapter().getSelectors().selectAll(State.Entities);
}

/**
 * Selects the current NPC count.
 * User Story: As NPC list consumers, I need a count selector so callers can
 * mirror RTK entity selector behavior without reading collection internals.
 */
inline int32 selectTotalNPCs(const FNPCSliceState &State) {
  return GetNPCAdapter().getSelectors().selectTotal(State.Entities);
}

/**
 * User Story: As NPC runtime state access, I need a selector for the active NPC
 * id so UI and orchestration logic can resolve the current actor consistently.
 * (From TS)
 */
inline FString selectActiveNpcId(const FNPCSliceState &State) {
  return State.ActiveNpcId;
}

/**
 * Selects the currently active NPC when one is set.
 * User Story: As NPC orchestration, I need the active NPC resolved so the
 * current actor can be processed without manual id lookups.
 */
inline func::Maybe<FNPCInternalState>
selectActiveNPC(const FNPCSliceState &State) {
  return State.ActiveNpcId.IsEmpty()
             ? func::nothing<FNPCInternalState>()
             : selectNPCById(State, State.ActiveNpcId);
}

} // namespace NPCSlice
