#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"

namespace MicroGame {

/** User Story: As a systems terminal transcript consumer, I need to invoke create transcript slice through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::Slice<FTranscriptState> CreateTranscriptSlice() */
inline rtk::Slice<FTranscriptState> CreateTranscriptSlice() {
  return rtk::createSlice<FTranscriptState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVEF9E8DE14021), FTranscriptState(),
      [](rtk::ActionReducerMapBuilder<FTranscriptState> &Builder) {
        Builder.addCase(
            TranscriptActions::recordTranscriptActionCreator(),
            [](const FTranscriptState &S,
               const rtk::Action<FTranscriptEntry> &A) -> FTranscriptState {
              return FTranscriptState{func::append_value<FTranscriptEntry>(
                  S.Entries, A.PayloadValue)};
            });
        Builder.addCase(
            TranscriptActions::resetTranscriptActionCreator(),
            [](const FTranscriptState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FTranscriptState {
              return FTranscriptState();
            });
      });
}

} // namespace MicroGame
