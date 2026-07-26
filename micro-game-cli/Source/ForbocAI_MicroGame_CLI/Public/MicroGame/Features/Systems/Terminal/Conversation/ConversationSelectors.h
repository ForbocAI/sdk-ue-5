#pragma once

#include "Features/API/Endpoints/NPC/Conversation/ConversationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame::ConversationSelectors {

/**
 * User Story: As a micro-game presenter, I need the latest NPC conversation CLI entry selected without mutating transcript state.
 * @fn inline func::Maybe<FTranscriptEntry> SelectConversationEntry( const TArray<FTranscriptEntry> &Entries, int32 Index)
 */
inline func::Maybe<FTranscriptEntry> SelectConversationEntry(
    const TArray<FTranscriptEntry> &Entries, int32 Index) {
  return Index < VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount
             ? func::nothing<FTranscriptEntry>()
         : Entries[Index].CommandGroup ==
               VerificationVocabularyAdapters::GameRuntimeData()
                   .commandGroups.npc_conversation
             ? func::just(Entries[Index])
             : SelectConversationEntry(
                   Entries,
                   Index - VerificationVocabularyAdapters::GameRuntimeData()
                               .numbers.nextIndex);
}

/** User Story: As a micro-game presenter, I need successful API conversation output decoded through the SDK codec. @fn inline func::Maybe<FNPCConversationResponse> SelectConversationResponse(const FTranscriptEntry &Entry) */
inline func::Maybe<FNPCConversationResponse>
SelectConversationResponse(const FTranscriptEntry &Entry) {
  FNPCConversationResponse Response;
  return APISlice::Detail::DecodeNpcConversationResponse(Entry.Output,
                                                          Response)
             ? func::just(Response)
             : func::nothing<FNPCConversationResponse>();
}

/** User Story: As a micro-game operator, I need only the first captured SDK CLI error line placed in the final transcript boundary. @fn inline FString SelectFirstConversationOutputLine(const FString &Output) */
inline FString SelectFirstConversationOutputLine(const FString &Output) {
  const FTerminalConversationTranscriptData &Data =
      TerminalAdapters::TerminalData().conversationTranscript;
  const int32 LineBreakIndex =
      Output.Find(VerificationAdapters::GameData().output.lineBreak);
  const FString FirstLine =
      LineBreakIndex == INDEX_NONE ? Output : Output.Left(LineBreakIndex);
  return FirstLine.IsEmpty() ? Data.FailureFallback : FirstLine;
}

/** User Story: As a micro-game operator, I need a failed SDK CLI conversation rendered from authored data without synthesizing participants or dialogue. @fn inline TArray<FTerminalLineViewModel> SelectConversationFailureViewModel(const FTranscriptEntry &Entry) */
inline TArray<FTerminalLineViewModel>
SelectConversationFailureViewModel(const FTranscriptEntry &Entry) {
  const FTerminalData &Terminal = TerminalAdapters::TerminalData();
  const TMap<FString, FString> Values =
      func::upsert_map_value<FString, FString>(
          {}, Terminal.tokens.output, FString(),
          [&Entry](const FString &) {
            return SelectFirstConversationOutputLine(Entry.Output);
          });
  return func::map_array<FTerminalAuthoredLine, FTerminalLineViewModel>(
      Terminal.conversationTranscript.FailureLines,
      [&Values](const FTerminalAuthoredLine &Line) {
        return FTerminalLineViewModel{
            Line.bError,
            VerificationAdapters::FormatGameTemplate(Line.Template, Values)};
      });
}

/**
 * User Story: As a micro-game presenter, I need API-authored transcript lines projected without conversation logic.
 * @fn inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel( const TArray<FTranscriptEntry> &Entries)
 */
inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel(
    const TArray<FTranscriptEntry> &Entries) {
  return func::match(
      SelectConversationEntry(
          Entries, Entries.Num() -
                       VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex),
      [](const FTranscriptEntry &Entry) {
        return func::match(
            SelectConversationResponse(Entry),
            [](const FNPCConversationResponse &Response) {
              return func::map_array<FString, FTerminalLineViewModel>(
                  Response.TranscriptLines, [](const FString &Line) {
                    return FTerminalLineViewModel{false, Line};
                  });
            },
            [&Entry]() {
              return Entry.Status == TerminalAdapters::TerminalData()
                                         .conversationTranscript.ErrorStatus
                         ? SelectConversationFailureViewModel(Entry)
                         : TArray<FTerminalLineViewModel>();
            });
      },
      []() { return TArray<FTerminalLineViewModel>(); });
}

} // namespace MicroGame::ConversationSelectors
