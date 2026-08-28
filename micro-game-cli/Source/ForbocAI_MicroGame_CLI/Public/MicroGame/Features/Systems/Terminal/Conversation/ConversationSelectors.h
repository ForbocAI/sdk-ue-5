#pragma once

#include "Dom/JsonObject.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MicroGame::ConversationSelectors {

namespace detail {

/**
 * User Story: As a conversation presenter, I need each API transcript value narrowed to display text without inventing dialogue.
 * @fn inline FString SelectConversationLineText( const TSharedPtr<FJsonValue> &Value, const FTerminalConversationTranscriptData &Data)
 */
inline FString SelectConversationLineText(
    const TSharedPtr<FJsonValue> &Value,
    const FTerminalConversationTranscriptData &Data) {
  FString Text;
  const bool bString = Value.IsValid() && Value->TryGetString(Text);
  const TSharedPtr<FJsonObject> Object =
      Value.IsValid() && Value->Type == EJson::Object
          ? Value->AsObject()
          : TSharedPtr<FJsonObject>();
  return bString
             ? Text
             : Object.IsValid() &&
                       Object->TryGetStringField(Data.LineTextField, Text)
                   ? Text
                   : Data.FailureFallback;
}

/**
 * User Story: As a conversation presenter, I need successful API conversation JSON decoded into its authored transcript-line projection.
 * @fn inline TArray<FTerminalLineViewModel> SelectSuccessfulConversationLines( const FTranscriptEntry &Entry, const FTerminalConversationTranscriptData &Data)
 */
inline TArray<FTerminalLineViewModel> SelectSuccessfulConversationLines(
    const FTranscriptEntry &Entry,
    const FTerminalConversationTranscriptData &Data) {
  TSharedPtr<FJsonObject> Object;
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Entry.Output);
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  return FJsonSerializer::Deserialize(Reader, Object) && Object.IsValid() &&
                 Object->TryGetArrayField(Data.TranscriptLinesField, Values) &&
                 Values != nullptr
             ? func::map_array<TSharedPtr<FJsonValue>,
                               FTerminalLineViewModel>(
                   *Values, [&Data](const TSharedPtr<FJsonValue> &Value) {
                     return FTerminalLineViewModel{
                         false, SelectConversationLineText(Value, Data)};
                   })
             : TArray<FTerminalLineViewModel>();
}

/**
 * User Story: As a conversation presenter, I need API failures mapped through authored lines while preserving the real CLI error.
 * @fn inline TArray<FTerminalLineViewModel> SelectFailedConversationLines( const FTranscriptEntry &Entry, const FTerminalData &Data)
 */
inline TArray<FTerminalLineViewModel> SelectFailedConversationLines(
    const FTranscriptEntry &Entry, const FTerminalData &Data) {
  TMap<FString, FString> Values;
  Values.Add(Data.tokens.output,
             Entry.Output.IsEmpty()
                 ? Data.conversationTranscript.FailureFallback
                 : Entry.Output);
  return func::map_array<FTerminalAuthoredLine, FTerminalLineViewModel>(
      Data.conversationTranscript.FailureLines,
      [&Values](const FTerminalAuthoredLine &Line) {
        return FTerminalLineViewModel{
            Line.bError,
            VerificationAdapters::FormatGameTemplate(Line.Template, Values)};
      });
}

/** User Story: As a conversation presenter, I need one transcript entry projected by success, failure, or opaque-response shape. @fn inline TArray<FTerminalLineViewModel> SelectConversationBlock(const FTranscriptEntry &Entry) */
inline TArray<FTerminalLineViewModel>
SelectConversationBlock(const FTranscriptEntry &Entry) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const TArray<FTerminalLineViewModel> Successful =
      SelectSuccessfulConversationLines(Entry, Data.conversationTranscript);
  return Successful.Num() >
                 VerificationVocabularyAdapters::GameRuntimeData()
                     .numbers.emptyCount
             ? Successful
             : Entry.Status == Data.conversationTranscript.ErrorStatus
                   ? SelectFailedConversationLines(Entry, Data)
                   : TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
                         false,
                         Entry.Output.IsEmpty()
                             ? Data.conversationTranscript.FailureFallback
                             : Entry.Output}};
}

} // namespace detail

/**
 * User Story: As a micro-game presenter, I need the client-orchestrated two-NPC chat turns replayed at the final transcript boundary from the real per-turn npc decide output, without synthesizing API data.
 * @fn inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel( const TArray<FTranscriptEntry> &Entries)
 */
inline TArray<FTerminalLineViewModel> SelectConversationTranscriptViewModel(
    const TArray<FTranscriptEntry> &Entries) {
  const FString Group = VerificationVocabularyAdapters::GameRuntimeData()
                            .commandGroups.npc_conversation;
  return func::fold_array<FTranscriptEntry,
                          TArray<FTerminalLineViewModel>>(
      func::filter_array<FTranscriptEntry>(
          Entries, [&Group](const FTranscriptEntry &Entry) {
            return Entry.CommandGroup == Group;
          }),
      {}, [](const TArray<FTerminalLineViewModel> &Lines,
             const FTranscriptEntry &Entry) {
        return func::append_values<FTerminalLineViewModel>(
            Lines, detail::SelectConversationBlock(Entry));
      });
}

} // namespace MicroGame::ConversationSelectors
