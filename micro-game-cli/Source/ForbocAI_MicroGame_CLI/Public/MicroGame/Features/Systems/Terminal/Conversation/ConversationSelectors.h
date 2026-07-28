#pragma once

#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame::ConversationSelectors {

/**
 * User Story: As a micro-game presenter, I need the client-orchestrated two-NPC chat turns replayed at the final transcript boundary from the real per-turn npc decide output, without synthesizing API data.
 * @fn inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel( const TArray<FTranscriptEntry> &Entries)
 */
inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel(
    const TArray<FTranscriptEntry> &Entries) {
  const FString Group = VerificationVocabularyAdapters::GameRuntimeData()
                            .commandGroups.npc_conversation;
  return func::map_array<FTranscriptEntry, FTerminalLineViewModel>(
      Entries.FilterByPredicate([&Group](const FTranscriptEntry &Entry) {
        return Entry.CommandGroup == Group;
      }),
      [](const FTranscriptEntry &Entry) {
        return FTerminalLineViewModel{false, Entry.Output};
      });
}

} // namespace MicroGame::ConversationSelectors
