#pragma once

#include "Features/API/Endpoints/NPC/Conversation/ConversationAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame::ConversationSelectors {

/**
 * User Story: As a test-game presenter, I need the latest API-authored conversation decoded from CLI evidence.
 * @fn inline func::Maybe<FNPCConversationResponse> SelectConversationResponse( const TArray<FTranscriptEntry> &Entries, int32 Index)
 */
inline func::Maybe<FNPCConversationResponse> SelectConversationResponse(
    const TArray<FTranscriptEntry> &Entries, int32 Index) {
  return Index < GameAdapters::GameRuntimeData().numbers.emptyCount
             ? func::nothing<FNPCConversationResponse>()
         : Entries[Index].CommandGroup ==
               GameAdapters::GameRuntimeData()
                   .commandGroups.npc_conversation
             ? [&]() {
                 FNPCConversationResponse Response;
                 return APISlice::Detail::DecodeNpcConversationResponse(
                            Entries[Index].Output, Response)
                            ? func::just(Response)
                            : func::nothing<FNPCConversationResponse>();
               }()
             : SelectConversationResponse(
                   Entries,
                   Index - GameAdapters::GameRuntimeData()
                               .numbers.nextIndex);
}

/**
 * User Story: As a test-game presenter, I need API-authored transcript lines projected without conversation logic.
 * @fn inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel( const TArray<FTranscriptEntry> &Entries)
 */
inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel(
    const TArray<FTranscriptEntry> &Entries) {
  return func::match(
      SelectConversationResponse(
          Entries, Entries.Num() -
                       GameAdapters::GameRuntimeData().numbers.nextIndex),
      [](const FNPCConversationResponse &Response) {
        return func::map_array<FString, FTerminalLineViewModel>(
            Response.TranscriptLines, [](const FString &Line) {
              return FTerminalLineViewModel{false, Line};
            });
      },
      []() { return TArray<FTerminalLineViewModel>(); });
}

} // namespace TestGame::ConversationSelectors
