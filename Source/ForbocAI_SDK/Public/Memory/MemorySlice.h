#pragma once
/**
 * memory lanes should read like ledger lines, not fog
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicMemoryMemorySliceHOptionalDomainId;
} } }

namespace MemorySlice {

using namespace rtk;
using namespace func;

namespace detail {

/**
 * Recursively collects memory item ids into an output array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void CollectIdsRecursive(const TArray<FMemoryItem> &Items,
                                TArray<FString> &Out, int32 Index) {
  Index < Items.Num()
      ? (Out.Add(Items[Index].Id),
         CollectIdsRecursive(Items, Out, Index + 1), void())
      : void();
}

/**
 * Recursively resolves memory ids into items, keeping only found entries.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
template <typename SelectorFn>
inline void ResolveIdsRecursive(const TArray<FString> &Ids,
                                TArray<FMemoryItem> &Out, int32 Index,
                                SelectorFn Selector) {
  Index < Ids.Num()
      ? [&]() {
          const Maybe<FMemoryItem> Item = Selector(Ids[Index]);
          Item.hasValue ? (Out.Add(Item.value), void()) : void();
          ResolveIdsRecursive(Ids, Out, Index + 1, Selector);
        }()
      : void();
}

} // namespace detail

/**
 * Returns the entity id for a memory item.
 * User Story: As memory entity adapters, I need a stable id selector so memory
 * items can be indexed and updated by identifier.
 */
inline FString MemoryItemIdSelector(const FMemoryItem &Item) { return Item.Id; }

/**
 * Returns the entity adapter used to manage memory records by id.
 * User Story: As memory reducers and selectors, I need one shared adapter so
 * entity operations stay consistent across the slice.
 */
inline EntityAdapter<FMemoryItem> GetMemoryAdapter() {
  return createEntityAdapter<FMemoryItem>(&MemoryItemIdSelector);
}

struct FMemorySliceState {
  EntityState<FMemoryItem> Entities;
  FString StorageStatus;
  FString RecallStatus;
  FString Error;
  TArray<FString> LastRecalledIds;

  FMemorySliceState()
      : Entities(GetMemoryAdapter().getInitialState()),
        StorageStatus(TEXT("idle")), RecallStatus(TEXT("idle")) {}
};

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

/**
 * Builds the memory slice reducer and extra cases.
 * User Story: As memory runtime setup, I need one slice factory so storage and
 * recall actions share a single reducer definition.
 */
inline Slice<FMemorySliceState> CreateMemorySlice() {
  return rtk::createSlice<FMemorySliceState>(
  TEXT("memory"), FMemorySliceState(),
  [](rtk::ActionReducerMapBuilder<FMemorySliceState> &Builder) {
    Builder.addCase(Actions::memoryStoreStartActionCreator(),
      [](const FMemorySliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FMemorySliceState {
                  FMemorySliceState Next = State;
                  Next.StorageStatus = TEXT("storing");
                  Next.Error.Empty();
                  return Next;
                });
    Builder.addCase(Actions::memoryStoreSuccessActionCreator(),
      [](const FMemorySliceState &State,
                             const Action<FMemoryItem> &Action) -> FMemorySliceState {
                            FMemorySliceState Next = State;
                            Next.StorageStatus = TEXT("idle");
                            Next.Entities = GetMemoryAdapter().upsertOne(
                                Next.Entities, Action.PayloadValue);
                            return Next;
                          });
    Builder.addCase(Actions::memoryStoreFailedActionCreator(),
      [](const FMemorySliceState &State,
                             const Action<FString> &Action) -> FMemorySliceState {
                            FMemorySliceState Next = State;
                            Next.StorageStatus = TEXT("error");
                            Next.Error = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::memoryRecallStartActionCreator(),
      [](const FMemorySliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FMemorySliceState {
                  FMemorySliceState Next = State;
                  Next.RecallStatus = TEXT("recalling");
                  Next.Error.Empty();
                  return Next;
                });
    Builder.addCase(Actions::memoryRecallSuccessActionCreator(),
      [](const FMemorySliceState &State,
                   const Action<TArray<FMemoryItem>> &Action) -> FMemorySliceState {
                  FMemorySliceState Next = State;
                  Next.RecallStatus = TEXT("idle");
                  Next.Entities = GetMemoryAdapter().upsertMany(Next.Entities,
                                                                Action.PayloadValue);
                  Next.LastRecalledIds.Empty(Action.PayloadValue.Num());
                  detail::CollectIdsRecursive(Action.PayloadValue,
                                              Next.LastRecalledIds, 0);
                  return Next;
                });
    Builder.addCase(Actions::memoryRecallFailedActionCreator(),
      [](const FMemorySliceState &State,
                             const Action<FString> &Action) -> FMemorySliceState {
                            FMemorySliceState Next = State;
                            Next.RecallStatus = TEXT("error");
                            Next.Error = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::memoryClearActionCreator(),
      [](const FMemorySliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FMemorySliceState {
                  return FMemorySliceState();
                });
  });
}

/**
 * Selects a single memory item by id.
 * User Story: As memory lookups, I need a direct selector so code can resolve
 * one memory record without scanning the full collection.
 */
inline Maybe<FMemoryItem> selectMemoryById(const FMemorySliceState &State,
                                           const FString &Id) {
  return GetMemoryAdapter().getSelectors().selectById(State.Entities, Id);
}

/**
 * Selects all memory items currently stored in the slice.
 * User Story: As memory inspection, I need the full memory collection so tools
 * and runtime systems can review current stored observations.
 */
inline TArray<FMemoryItem> selectAllMemories(const FMemorySliceState &State) {
  return GetMemoryAdapter().getSelectors().selectAll(State.Entities);
}

/**
 * User Story: As recall-result consumers, I need a selector that resolves the
 * last recalled memory ids into entities for immediate post-recall rendering.
 * (From TS)
 */
inline TArray<FMemoryItem>
selectLastRecalledMemories(const FMemorySliceState &State) {
  TArray<FMemoryItem> Results;
  detail::ResolveIdsRecursive(
      State.LastRecalledIds, Results, 0,
      [&State](const FString &Id) { return selectMemoryById(State, Id); });
  return Results;
}

} // namespace MemorySlice
