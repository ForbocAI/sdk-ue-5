#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"

namespace TestGame {

inline rtk::Slice<FTranscriptState> CreateTranscriptSlice() {
  return rtk::createSlice<FTranscriptState>(
      TEXT("testgame/transcript"), FTranscriptState(),
      [](rtk::ActionReducerMapBuilder<FTranscriptState> &Builder) {
        Builder.addCase(
            TranscriptActions::recordTranscriptActionCreator(),
            [](const FTranscriptState &S,
               const rtk::Action<TranscriptActions::FRecordTranscriptPayload>
                   &A) -> FTranscriptState {
              FTranscriptState Next = S;
              FTranscriptEntry Entry;
              Entry.Id = FString::Printf(TEXT("%lld-%d"),
                                         FDateTime::Now().GetTicks(),
                                         FMath::Rand());
              Entry.ScenarioId = A.PayloadValue.ScenarioId;
              Entry.CommandGroup = A.PayloadValue.CommandGroup;
              Entry.Command = A.PayloadValue.Command;
              Entry.ExpectedRoutes = A.PayloadValue.ExpectedRoutes;
              Entry.Status = A.PayloadValue.Status;
              Entry.Output = A.PayloadValue.Output;
              Entry.Timestamp = FDateTime::Now().ToIso8601();
              Next.Entries.Add(Entry);
              return Next;
            });
        Builder.addCase(
            TranscriptActions::resetTranscriptActionCreator(),
            [](const FTranscriptState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FTranscriptState {
              return FTranscriptState();
            });
      });
}

} // namespace TestGame
