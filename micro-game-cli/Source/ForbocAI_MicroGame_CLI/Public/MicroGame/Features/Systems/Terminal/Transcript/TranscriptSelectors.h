#pragma once

#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
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
             ? VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount
             : ((Entries[Index].Status ==
                         VerificationVocabularyAdapters::GameRuntimeData().statuses.error
                     ? VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex
                     : VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount) +
                SelectTranscriptErrorCountRecursive(
                    Entries,
                    Index + VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex));
}

/** User Story: As a systems terminal transcript consumer, I need to invoke select transcript error count through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline int32 SelectTranscriptErrorCount(const FTranscriptState &State) */
inline int32 SelectTranscriptErrorCount(const FTranscriptState &State) {
  return SelectTranscriptErrorCountRecursive(
      State.Entries, VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount);
}

} // namespace TranscriptSelectors
} // namespace MicroGame
