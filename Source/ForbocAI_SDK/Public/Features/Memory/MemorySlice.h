#pragma once
/**
 * memory lanes should read like ledger lines, not fog
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"
#include "Features/Memory/MemoryActions.h"
#include "Features/Memory/MemoryAdapters.h"

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

/**
 * Builds the memory slice reducer and extra cases.
 * User Story: As memory runtime setup, I need one slice factory so storage and
 * recall actions share a single reducer definition.
 */
inline Slice<FMemorySliceState> createMemorySlice() {
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
