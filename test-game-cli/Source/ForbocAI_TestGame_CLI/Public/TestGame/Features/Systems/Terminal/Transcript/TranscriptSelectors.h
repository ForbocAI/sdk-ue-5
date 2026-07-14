#pragma once

#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace TranscriptSelectors {

inline TArray<FTranscriptEntry>
SelectTranscriptEntries(const FTranscriptState &State) {
  return State.Entries;
}

inline int32 SelectTranscriptErrorCountRecursive(
    const TArray<FTranscriptEntry> &Entries, int32 Index) {
  return Index >= Entries.Num()
             ? 0
             : ((Entries[Index].Status == ETranscriptStatus::Error ? 1 : 0) +
                SelectTranscriptErrorCountRecursive(Entries, Index + 1));
}

inline int32 SelectTranscriptErrorCount(const FTranscriptState &State) {
  return SelectTranscriptErrorCountRecursive(State.Entries, 0);
}

} // namespace TranscriptSelectors
} // namespace TestGame
