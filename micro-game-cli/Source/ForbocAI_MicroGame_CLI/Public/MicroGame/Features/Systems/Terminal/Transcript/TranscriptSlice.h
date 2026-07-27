#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
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
               const rtk::Action<TranscriptActions::FRecordTranscriptPayload>
                   &A) -> FTranscriptState {
              FTranscriptState Next = S;
              FTranscriptEntry Entry;
              Entry.Id = FString::Format(
                  *TerminalAdapters::TerminalData().transcript.idFormat,
                  {FDateTime::Now().GetTicks(), FMath::Rand()});
              Entry.ScenarioId = A.PayloadValue.ScenarioId;
              Entry.CommandGroup = A.PayloadValue.CommandGroup;
              Entry.Command = A.PayloadValue.Command;
              Entry.ExpectedRoutes = A.PayloadValue.ExpectedRoutes;
              Entry.Status = A.PayloadValue.Status;
              Entry.Output = A.PayloadValue.Output;
              Entry.DurationMs = A.PayloadValue.DurationMs;
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

} // namespace MicroGame
