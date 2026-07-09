#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

#include "NPC/NPCTypes.h"

namespace NPCSlice {

struct FSetNPCStatePayload {
  FString Id;
  FAgentState State;
};

struct FUpdateNPCStatePayload {
  FString Id;
  FAgentState Delta;
};

struct FAddToHistoryPayload {
  FString Id;
  FString Role;
  FString Content;
};

struct FSetHistoryPayload {
  FString Id;
  TArray<FNPCHistoryEntry> History;
};

struct FSetLastActionPayload {
  FString Id;
  FAgentAction Action;
  bool bHasAction;
};

struct FBlockActionPayload {
  FString Id;
  FString Reason;
};

namespace Actions {

/**
 * Returns the memoized action creator for inserting or replacing NPC info.
 * User Story: As NPC lifecycle flows, I need a cached action creator so new or
 * updated NPC records dispatch through one contract.
 */
inline const rtk::ActionCreator<FNPCInternalState> &setNPCInfoActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FNPCInternalState>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FNPCInternalState> {
        return rtk::createAction<FNPCInternalState>(TEXT("npc/setNPCInfo"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for switching the active NPC id.
 * User Story: As active-NPC selection, I need a cached action creator so the
 * current actor can be changed consistently.
 */
inline const rtk::ActionCreator<FString> &setActiveNPCActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FString>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FString> {
        return rtk::createAction<FString>(TEXT("npc/setActiveNPC"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for replacing NPC state.
 * User Story: As NPC state updates, I need a cached action creator so full
 * state replacement uses one action contract.
 */
inline const rtk::ActionCreator<FSetNPCStatePayload> &
setNPCStateActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FSetNPCStatePayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FSetNPCStatePayload> {
        return rtk::createAction<FSetNPCStatePayload>(TEXT("npc/setNPCState"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for merging an NPC state delta.
 * User Story: As NPC delta updates, I need a cached action creator so partial
 * state changes can be dispatched consistently.
 */
inline const rtk::ActionCreator<FUpdateNPCStatePayload> &
updateNPCStateActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FUpdateNPCStatePayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FUpdateNPCStatePayload> {
        return rtk::createAction<FUpdateNPCStatePayload>(TEXT("npc/updateNPCState"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for appending NPC history.
 * User Story: As conversation tracking, I need a cached action creator so
 * history entries are added through one reducer contract.
 */
inline const rtk::ActionCreator<FAddToHistoryPayload> &
addToHistoryActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FAddToHistoryPayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FAddToHistoryPayload> {
        return rtk::createAction<FAddToHistoryPayload>(TEXT("npc/addToHistory"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for replacing NPC history.
 * User Story: As history synchronization, I need a cached action creator so
 * full transcript replacement uses a shared contract.
 */
inline const rtk::ActionCreator<FSetHistoryPayload> &setHistoryActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FSetHistoryPayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FSetHistoryPayload> {
        return rtk::createAction<FSetHistoryPayload>(TEXT("npc/setHistory"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for recording the last NPC action.
 * User Story: As action tracking, I need a cached action creator so the last
 * emitted action can be updated consistently.
 */
inline const rtk::ActionCreator<FSetLastActionPayload> &
setLastActionActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FSetLastActionPayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FSetLastActionPayload> {
        return rtk::createAction<FSetLastActionPayload>(TEXT("npc/setLastAction"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for blocking an NPC action.
 * User Story: As safety and rules enforcement, I need a cached action creator
 * so block reasons are stored through one contract.
 */
inline const rtk::ActionCreator<FBlockActionPayload> &
blockActionActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FBlockActionPayload>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FBlockActionPayload> {
        return rtk::createAction<FBlockActionPayload>(TEXT("npc/blockAction"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for clearing NPC action blocking.
 * User Story: As unblock flows, I need a cached action creator so block state
 * can be removed consistently when conditions change.
 */
inline const rtk::ActionCreator<FString> &clearBlockActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FString>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FString> {
        return rtk::createAction<FString>(TEXT("npc/clearBlock"));
      });
  return func::eval(ActionCreator);
}

/**
 * Returns the memoized action creator for removing an NPC from the slice.
 * User Story: As NPC teardown, I need a cached action creator so removals
 * propagate through one shared reducer contract.
 */
inline const rtk::ActionCreator<FString> &removeNPCActionCreator() {
  static const func::Lazy<rtk::ActionCreator<FString>> ActionCreator =
      func::lazy([]() -> rtk::ActionCreator<FString> {
        return rtk::createAction<FString>(TEXT("npc/removeNPC"));
      });
  return func::eval(ActionCreator);
}

/**
 * Builds the action that inserts or replaces NPC info.
 * User Story: As NPC lifecycle flows, I need a simple action factory so NPC
 * records can be inserted or refreshed without manual payload wiring.
 */
inline rtk::AnyAction setNPCInfo(const FNPCInternalState &Info) {
  return setNPCInfoActionCreator()(Info);
}

/**
 * Builds the action that switches the active NPC id.
 * User Story: As actor selection, I need an action factory so the runtime can
 * mark one NPC as active through a single helper.
 */
inline rtk::AnyAction setActiveNPC(const FString &Id) {
  return setActiveNPCActionCreator()(Id);
}

/**
 * Builds the action that replaces an NPC state payload.
 * User Story: As full state updates, I need an action factory so entire NPC
 * state blobs can be dispatched consistently.
 */
inline rtk::AnyAction setNPCState(const FString &Id, const FAgentState &State) {
  return setNPCStateActionCreator()(FSetNPCStatePayload{Id, State});
}

/**
 * Builds the action that merges an NPC state delta.
 * User Story: As delta updates, I need an action factory so partial NPC state
 * changes can be dispatched without manual payload structs.
 */
inline rtk::AnyAction updateNPCState(const FString &Id,
                                     const FAgentState &Delta) {
  return updateNPCStateActionCreator()(FUpdateNPCStatePayload{Id, Delta});
}

/**
 * Builds the action that appends an entry to NPC history.
 * User Story: As conversation logging, I need an action factory so new history
 * entries are recorded with one helper call.
 */
inline rtk::AnyAction addToHistory(const FString &Id, const FString &Role,
                                   const FString &Content) {
  return addToHistoryActionCreator()(FAddToHistoryPayload{Id, Role, Content});
}

/**
 * Builds the action that replaces the full NPC history.
 * User Story: As transcript synchronization, I need an action factory so
 * complete history sets can be applied consistently.
 */
inline rtk::AnyAction setHistory(const FString &Id,
                                 const TArray<FNPCHistoryEntry> &History) {
  return setHistoryActionCreator()(FSetHistoryPayload{Id, History});
}

/**
 * Builds the action that records the last NPC action and presence flag.
 * User Story: As last-action tracking, I need an action factory so reducers can
 * update whether an NPC has a recent action in one dispatch.
 */
inline rtk::AnyAction setLastAction(const FString &Id,
                                    const FAgentAction &LastAction,
                                    bool bHasAction = true) {
  return setLastActionActionCreator()(
      FSetLastActionPayload{Id, LastAction, bHasAction});
}

/**
 * Builds the action that blocks an NPC action with a reason.
 * User Story: As rule enforcement, I need an action factory so block reasons
 * can be applied consistently to the targeted NPC.
 */
inline rtk::AnyAction blockAction(const FString &Id, const FString &Reason) {
  return blockActionActionCreator()(FBlockActionPayload{Id, Reason});
}

/**
 * Builds the action that clears blocking for an NPC.
 * User Story: As unblock flows, I need an action factory so NPC block state can
 * be removed without custom payload assembly.
 */
inline rtk::AnyAction clearBlock(const FString &Id) {
  return clearBlockActionCreator()(Id);
}

/**
 * Builds the action that removes an NPC from the slice.
 * User Story: As NPC teardown, I need an action factory so removal dispatches
 * stay consistent across runtime, tests, and tools.
 */
inline rtk::AnyAction removeNPC(const FString &Id) {
  return removeNPCActionCreator()(Id);
}

} // namespace Actions

} // namespace NPCSlice
