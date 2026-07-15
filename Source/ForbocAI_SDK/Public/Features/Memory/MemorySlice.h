#pragma once

#include "Core/rtk.hpp"
#include "Features/Memory/MemoryActions.h"
#include "Features/Memory/MemoryAdapters.h"

namespace MemorySlice {

struct FMemorySliceState {
  rtk::EntityState<FMemoryItem> Entities;
  FString StorageStatus;
  FString RecallStatus;
  FString Error;
  TArray<FString> LastRecalledIds;

  FMemorySliceState()
      : Entities(GetMemoryAdapter().getInitialState()),
        StorageStatus(TEXT("idle")), RecallStatus(TEXT("idle")) {}
};

inline void CollectMemoryIds(const TArray<FMemoryItem> &Items,
                             TArray<FString> &Ids, int32 Index) {
  Index >= Items.Num()
      ? void()
      : (Ids.Add(Items[Index].Id),
         CollectMemoryIds(Items, Ids, Index + 1), void());
}

inline rtk::Slice<FMemorySliceState> createMemorySlice() {
  return rtk::createSlice<FMemorySliceState>(
      TEXT("memory"), FMemorySliceState(),
      [](rtk::ActionReducerMapBuilder<FMemorySliceState> &Builder) {
        Builder.addCase(
            Actions::memoryStoreStartActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FMemorySliceState Next = State;
              Next.StorageStatus = TEXT("storing");
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            Actions::memoryStoreSuccessActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<FMemoryItem> &Action) {
              FMemorySliceState Next = State;
              Next.StorageStatus = TEXT("idle");
              Next.Error.Empty();
              Next.Entities = GetMemoryAdapter().upsertOne(
                  Next.Entities, Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            Actions::memoryStoreFailedActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<FString> &Action) {
              FMemorySliceState Next = State;
              Next.StorageStatus = TEXT("error");
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallStartActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FMemorySliceState Next = State;
              Next.RecallStatus = TEXT("recalling");
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallSuccessActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<TArray<FMemoryItem>> &Action) {
              FMemorySliceState Next = State;
              Next.RecallStatus = TEXT("idle");
              Next.Error.Empty();
              Next.Entities = GetMemoryAdapter().upsertMany(
                  Next.Entities, Action.PayloadValue);
              Next.LastRecalledIds.Empty(Action.PayloadValue.Num());
              CollectMemoryIds(Action.PayloadValue, Next.LastRecalledIds, 0);
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallFailedActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<FString> &Action) {
              FMemorySliceState Next = State;
              Next.RecallStatus = TEXT("error");
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::memoryClearActionCreator(),
            [](const FMemorySliceState &,
               const rtk::Action<rtk::FEmptyPayload> &) {
              return FMemorySliceState();
            });
      });
}

} // namespace MemorySlice
