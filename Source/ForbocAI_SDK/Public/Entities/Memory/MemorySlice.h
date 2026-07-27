#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Entities/Memory/MemoryActions.h"
#include "Systems/Memory/MemoryAdapters.h"

namespace MemorySlice {

/**
 * User Story: As memory initialization, I need entity state and authored
 * lifecycle defaults composed at the slice boundary.
 * @fn inline FMemorySliceState createMemoryInitialState()
 */
inline FMemorySliceState createMemoryInitialState() {
  FMemorySliceState State;
  State.Entities = GetMemoryAdapter().getInitialState();
  State.StorageStatus = MemoryConfiguration::memoryData().Status.Idle;
  State.RecallStatus = MemoryConfiguration::memoryData().Status.Idle;
  return State;
}

/** User Story: As a features memory consumer, I need to invoke create memory slice through a stable signature so the features memory workflow remains explicit and composable. @fn inline rtk::Slice<FMemorySliceState> createMemorySlice() */
inline rtk::Slice<FMemorySliceState> createMemorySlice() {
  const FMemorySliceState InitialState = createMemoryInitialState();
  return rtk::createSlice<FMemorySliceState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3406C51E0F42), InitialState,
      [InitialState](rtk::ActionReducerMapBuilder<FMemorySliceState> &Builder) {
        Builder.addCase(
            Actions::memoryStoreStartActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FMemorySliceState Next = State;
              Next.StorageStatus =
                  MemoryConfiguration::memoryData().Status.Storing;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            Actions::memoryStoreSuccessActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<FMemoryItem> &Action) {
              FMemorySliceState Next = State;
              Next.StorageStatus =
                  MemoryConfiguration::memoryData().Status.Idle;
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
              Next.StorageStatus =
                  MemoryConfiguration::memoryData().Status.Error;
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallStartActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FMemorySliceState Next = State;
              Next.RecallStatus =
                  MemoryConfiguration::memoryData().Status.Recalling;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallSuccessActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<TArray<FMemoryItem>> &Action) {
              FMemorySliceState Next = State;
              Next.RecallStatus =
                  MemoryConfiguration::memoryData().Status.Idle;
              Next.Error.Empty();
              Next.Entities = GetMemoryAdapter().upsertMany(
                  Next.Entities, Action.PayloadValue);
              Next.RecalledIds = func::map_array<FMemoryItem, FString>(
                  Action.PayloadValue,
                  [](const FMemoryItem &Item) { return Item.Id; });
              return Next;
            });
        Builder.addCase(
            Actions::memoryRecallFailedActionCreator(),
            [](const FMemorySliceState &State,
               const rtk::Action<FString> &Action) {
              FMemorySliceState Next = State;
              Next.RecallStatus =
                  MemoryConfiguration::memoryData().Status.Error;
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::memoryClearActionCreator(),
            [InitialState](const FMemorySliceState &,
               const rtk::Action<rtk::FEmptyPayload> &) {
              return InitialState;
            });
      });
}

} // namespace MemorySlice
