#pragma once
/**
 * Memory action creators own memory event construction.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

namespace MemorySlice {

using namespace rtk;
using namespace func;

namespace Actions {

/**
 * Returns the memoized action creator for memory-store start events.
 * User Story: As memory storage flows, I need a cached action creator so every
 * caller dispatches the same pending action contract.
 */
inline const ActionCreatorWithoutPayload &memoryStoreStartActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("memory/memoryStoreStart"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for successful memory-store events.
 * User Story: As memory storage flows, I need a cached success action creator
 * so stored items enter the slice through one contract.
 */
inline const ActionCreator<FMemoryItem> &memoryStoreSuccessActionCreator() {
  static const ActionCreator<FMemoryItem> ActionCreator =
      createAction<FMemoryItem>(TEXT("memory/memoryStoreSuccess"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for failed memory-store events.
 * User Story: As memory error handling, I need a cached failure action creator
 * so storage errors can be surfaced consistently.
 */
inline const ActionCreator<FString> &memoryStoreFailedActionCreator() {
  static const ActionCreator<FString> ActionCreator =
      createAction<FString>(TEXT("memory/memoryStoreFailed"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for memory-recall start events.
 * User Story: As recall flows, I need a cached pending action creator so
 * recall state transitions stay uniform across callers.
 */
inline const ActionCreatorWithoutPayload &memoryRecallStartActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("memory/memoryRecallStart"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for successful memory recalls.
 * User Story: As recall flows, I need a cached success action creator so
 * recalled items are stored through one reducer contract.
 */
inline const ActionCreator<TArray<FMemoryItem>> &
memoryRecallSuccessActionCreator() {
  static const ActionCreator<TArray<FMemoryItem>> ActionCreator =
      createAction<TArray<FMemoryItem>>(TEXT("memory/memoryRecallSuccess"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for failed memory recalls.
 * User Story: As recall error handling, I need a cached failure action creator
 * so recall problems are represented consistently in slice state.
 */
inline const ActionCreator<FString> &memoryRecallFailedActionCreator() {
  static const ActionCreator<FString> ActionCreator =
      createAction<FString>(TEXT("memory/memoryRecallFailed"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for clearing memory state.
 * User Story: As cleanup flows, I need a cached clear action creator so memory
 * state can be reset through a single action contract.
 */
inline const ActionCreatorWithoutPayload &memoryClearActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("memory/memoryClear"));
  return ActionCreator;
}

/**
 * Builds the action that marks remote or local memory storage as in flight.
 * User Story: As memory status tracking, I need pending actions so the UI can
 * reflect that a store operation has started.
 */
inline AnyAction memoryStoreStart() {
  return memoryStoreStartActionCreator()();
}

/**
 * Builds the action that records a successfully stored memory item.
 * User Story: As storage completion handling, I need stored items captured so
 * later queries can immediately see new memories.
 */
inline AnyAction memoryStoreSuccess(const FMemoryItem &Item) {
  return memoryStoreSuccessActionCreator()(Item);
}

/**
 * Builds the action that records a memory-store failure message.
 * User Story: As storage error handling, I need failure messages preserved so
 * callers can explain why a memory write failed.
 */
inline AnyAction memoryStoreFailed(const FString &Error) {
  return memoryStoreFailedActionCreator()(Error);
}

/**
 * Builds the action that marks memory recall as in flight.
 * User Story: As recall status tracking, I need pending actions so consumers
 * know recall results have not arrived yet.
 */
inline AnyAction memoryRecallStart() {
  return memoryRecallStartActionCreator()();
}

/**
 * Builds the action that records a successful memory recall result set.
 * User Story: As recall completion handling, I need recalled items stored so
 * the latest retrieval can be rendered and reused.
 */
inline AnyAction memoryRecallSuccess(const TArray<FMemoryItem> &Items) {
  return memoryRecallSuccessActionCreator()(Items);
}

/**
 * Builds the action that records a memory-recall failure message.
 * User Story: As recall error handling, I need failure messages stored so UI
 * and logs can explain why recall did not complete.
 */
inline AnyAction memoryRecallFailed(const FString &Error) {
  return memoryRecallFailedActionCreator()(Error);
}

/**
 * Builds the action that clears memory slice state.
 * User Story: As cleanup flows, I need a clear action so memory state can be
 * reset before switching NPC context or rerunning tests.
 */
inline AnyAction memoryClear() { return memoryClearActionCreator()(); }

} // namespace Actions

} // namespace MemorySlice
