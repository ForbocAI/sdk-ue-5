#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Systems/Vector/VectorThunks.h"
#include "Components/Vector/VectorTypes.h"

namespace VectorSlice {

/** User Story: As a features vector consumer, I need to invoke create vector slice through a stable signature so the features vector workflow remains explicit and composable. @fn inline rtk::Slice<FVectorState> createVectorSlice() */
inline rtk::Slice<FVectorState> createVectorSlice() {
  return rtk::createSlice<FVectorState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3DB7DF094BA0), FVectorState(),
      [](rtk::ActionReducerMapBuilder<FVectorState> &Builder) {
        Builder.addCase(
            rtk::initVectorThunk().pending,
            [](const FVectorState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FVectorState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV29762FEE4E50);
              Next.bIsReady = false;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            rtk::initVectorThunk().fulfilled,
            [](const FVectorState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FVectorState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV05D2DE39A21B);
              Next.bIsReady = true;
              Next.Error.Empty();
              return Next;
            });
        Builder.addCase(
            rtk::initVectorThunk().rejected,
            [](const FVectorState &State,
               const rtk::Action<FString> &Action) {
              FVectorState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
              Next.bIsReady = false;
              Next.Error = Action.PayloadValue;
              return Next;
            });
      });
}

} // namespace VectorSlice
