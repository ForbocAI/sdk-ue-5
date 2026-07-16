#pragma once

#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace TranscriptSelectors {

/** User Story: As a systems terminal transcript consumer, I need to invoke select transcript entries through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline TArray<FTranscriptEntry> SelectTranscriptEntries(const FTranscriptState &State) */
inline TArray<FTranscriptEntry>
SelectTranscriptEntries(const FTranscriptState &State) {
  return State.Entries;
}

/** User Story: As a systems terminal transcript consumer, I need to invoke select transcript error count recursive through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline int32 SelectTranscriptErrorCountRecursive( const TArray<FTranscriptEntry> &Entries, int32 Index) */
inline int32 SelectTranscriptErrorCountRecursive(
    const TArray<FTranscriptEntry> &Entries, int32 Index) {
  return Index >= Entries.Num()
             ? 0
             : ((Entries[Index].Status == ETranscriptStatus::Error ? 1 : 0) +
                SelectTranscriptErrorCountRecursive(Entries, Index + 1));
}

/** User Story: As a systems terminal transcript consumer, I need to invoke select transcript error count through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline int32 SelectTranscriptErrorCount(const FTranscriptState &State) */
inline int32 SelectTranscriptErrorCount(const FTranscriptState &State) {
  return SelectTranscriptErrorCountRecursive(State.Entries, 0);
}

} // namespace TranscriptSelectors
} // namespace TestGame
