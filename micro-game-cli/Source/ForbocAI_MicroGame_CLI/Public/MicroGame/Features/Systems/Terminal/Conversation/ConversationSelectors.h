#pragma once

#include "Core/JsonInterop/Fields/FieldsAdapters.h"
#include "Dom/JsonObject.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MicroGame::ConversationSelectors {

namespace detail {

/** User Story: As a conversation presenter, I need transcript objects parsed into Maybe so invalid JSON cannot cross the selector boundary. @fn inline func::Maybe<TSharedPtr<FJsonObject>> ParseConversationObject(const FString &Json) */
inline func::Maybe<TSharedPtr<FJsonObject>>
ParseConversationObject(const FString &Json) {
  TSharedPtr<FJsonObject> Object;
  const bool bParsed = JsonInterop::ParseJsonObject(Json, Object);
  return func::maybe_filter(
      func::from_shared(Object),
      [bParsed](const TSharedPtr<FJsonObject> &) { return bParsed; });
}

/**
 * User Story: As a conversation presenter, I need each API transcript value narrowed to display text without inventing dialogue.
 * @fn inline FString SelectConversationLineText( const TSharedPtr<FJsonValue> &Value, const FTerminalConversationTranscriptData &Data)
 */
inline FString SelectConversationLineText(
    const TSharedPtr<FJsonValue> &Value,
    const FTerminalConversationTranscriptData &Data) {
  const func::Maybe<TSharedPtr<FJsonValue>> Present =
      func::from_shared(Value);
  const func::Maybe<FString> Direct = func::fmap(
      func::maybe_filter(
          Present, [](const TSharedPtr<FJsonValue> &Candidate) {
            return Candidate->Type == EJson::String;
          }),
      [](const TSharedPtr<FJsonValue> &Candidate) {
        return Candidate->AsString();
      });
  const func::Maybe<FString> Structured = func::mbind(
      func::maybe_filter(
          Present, [](const TSharedPtr<FJsonValue> &Candidate) {
            return Candidate->Type == EJson::Object;
          }),
      [&Data](const TSharedPtr<FJsonValue> &Candidate) {
        return JsonInterop::StringFieldValue(Candidate->AsObject(),
                                             Data.LineTextField);
      });
  return func::match(
      Direct, [](const FString &Text) { return Text; },
      [&Structured, &Data]() {
        return func::or_else(Structured, Data.FailureFallback);
      });
}

/**
 * User Story: As a conversation presenter, I need successful API conversation JSON decoded into its authored transcript-line projection.
 * @fn inline TArray<FTerminalLineViewModel> SelectSuccessfulConversationLines( const FTranscriptEntry &Entry, const FTerminalConversationTranscriptData &Data)
 */
inline TArray<FTerminalLineViewModel> SelectSuccessfulConversationLines(
    const FTranscriptEntry &Entry,
    const FTerminalConversationTranscriptData &Data) {
  const func::Maybe<TArray<TSharedPtr<FJsonValue>>> Values = func::mbind(
      ParseConversationObject(Entry.Output),
      [&Data](const TSharedPtr<FJsonObject> &Object) {
        return JsonInterop::ArrayFieldValues(Object,
                                             Data.TranscriptLinesField);
      });
  return func::match(
      Values,
      [&Data](const TArray<TSharedPtr<FJsonValue>> &Present) {
        return func::map_array<TSharedPtr<FJsonValue>,
                               FTerminalLineViewModel>(
            Present, [&Data](const TSharedPtr<FJsonValue> &Value) {
              return FTerminalLineViewModel{
                  false, SelectConversationLineText(Value, Data)};
            });
      },
      []() { return TArray<FTerminalLineViewModel>(); });
}

/**
 * User Story: As a conversation presenter, I need API failures mapped through authored lines while preserving the real CLI error.
 * @fn inline TArray<FTerminalLineViewModel> SelectFailedConversationLines( const FTranscriptEntry &Entry, const FTerminalData &Data)
 */
inline TArray<FTerminalLineViewModel> SelectFailedConversationLines(
    const FTranscriptEntry &Entry, const FTerminalData &Data) {
  const TMap<FString, FString> Values{
      {Data.tokens.output,
       Entry.Output.IsEmpty() ? Data.conversationTranscript.FailureFallback
                              : Entry.Output}};
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
