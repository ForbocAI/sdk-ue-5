#pragma once

#include "Core/rtk.hpp"
#include "Features/Vector/VectorThunks.h"
#include "Features/Vector/VectorTypes.h"

namespace VectorSlice {

inline rtk::Slice<FVectorState> createVectorSlice() {
  return rtk::createSlice<FVectorState>(
      TEXT("vector"), FVectorState(),
      [](rtk::ActionReducerMapBuilder<FVectorState> &Builder) {
        Builder.addCase(
            rtk::initVectorThunk().pending,
            [](const FVectorState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FVectorState Next = State;
              Next.Status = TEXT("initializing");
              Next.bIsReady = false;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            rtk::initVectorThunk().fulfilled,
            [](const FVectorState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FVectorState Next = State;
              Next.Status = TEXT("ready");
              Next.bIsReady = true;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            rtk::initVectorThunk().rejected,
            [](const FVectorState &State,
               const rtk::Action<FString> &Action) {
              FVectorState Next = State;
              Next.Status = TEXT("failed");
              Next.bIsReady = false;
              Next.Error = Action.PayloadValue;
              return Next;
            });
      });
}

} // namespace VectorSlice
