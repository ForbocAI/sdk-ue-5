#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame::TerminalAdapters {

namespace detail {

/** User Story: As a terminal presenter, I need NPC glyph entries mapped from authored values so adding an entity marker requires data rather than selector code. @fn inline TArray<FTerminalNpcGlyph> ReadNpcGlyphs( const TArray<TSharedPtr<FJsonValue>> &Values) */
inline TArray<FTerminalNpcGlyph> ReadNpcGlyphs(
    const TArray<TSharedPtr<FJsonValue>> &Values) {
  return func::map_array<TSharedPtr<FJsonValue>, FTerminalNpcGlyph>(
      Values, [](const TSharedPtr<FJsonValue> &Value) {
        const TSharedRef<FJsonObject> Object =
            Value->AsObject().ToSharedRef();
        return FTerminalNpcGlyph{
            DataAdapters::ReadStringField(Object, TEXT("id")),
            DataAdapters::ReadStringField(Object, TEXT("glyph"))};
      });
}

/** User Story: As a terminal presenter, I need authored failure lines decoded as typed view data so selectors contain no embedded conversation prose. @fn inline TArray<FTerminalAuthoredLine> ReadAuthoredLines(const TArray<TSharedPtr<FJsonValue>> &Values) */
inline TArray<FTerminalAuthoredLine>
ReadAuthoredLines(const TArray<TSharedPtr<FJsonValue>> &Values) {
  return func::map_array<TSharedPtr<FJsonValue>, FTerminalAuthoredLine>(
      Values, [](const TSharedPtr<FJsonValue> &Value) {
        const TSharedRef<FJsonObject> Object =
            Value->AsObject().ToSharedRef();
        return FTerminalAuthoredLine{
            DataAdapters::ReadStringField(Object, TEXT("template")),
            DataAdapters::ReadBooleanField(Object, TEXT("error"))};
      });
}

} // namespace detail

/** User Story: As a terminal consumer, I need initial UI state and listener templates decoded from authored data so terminal roles contain no embedded presentation content. @fn inline FTerminalData ReadTerminalData() */
inline FTerminalData ReadTerminalData() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/terminal.json"));
  const TSharedRef<FJsonObject> InitialState =
      DataAdapters::ReadObjectField(Source, TEXT("initialState"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  const TSharedRef<FJsonObject> Grid =
      DataAdapters::ReadObjectField(Source, TEXT("grid"));
  const TSharedRef<FJsonObject> Levels =
      DataAdapters::ReadObjectField(Source, TEXT("levels"));
  const TSharedRef<FJsonObject> Tokens =
      DataAdapters::ReadObjectField(Source, TEXT("tokens"));
  const TSharedRef<FJsonObject> Transcript =
      DataAdapters::ReadObjectField(Source, TEXT("transcript"));
  const TSharedRef<FJsonObject> ConversationTranscript =
      DataAdapters::ReadObjectField(Source, TEXT("conversationTranscript"));
  FTerminalData Data;
  Data.initialState.Mode =
      DataAdapters::ReadStringField(InitialState, TEXT("mode"));
  Data.initialState.Messages =
      DataAdapters::ReadStringArrayField(InitialState, TEXT("messages"));
  Data.messages.npcMoved =
      DataAdapters::ReadStringField(Messages, TEXT("npcMoved"));
#define FORBOCAI_READ_TERMINAL_MESSAGE(Name)                              \
  Data.messages.Name = DataAdapters::ReadStringField(Messages, TEXT(#Name));
  FORBOCAI_READ_TERMINAL_MESSAGE(legend)
  FORBOCAI_READ_TERMINAL_MESSAGE(transcriptEntry)
  FORBOCAI_READ_TERMINAL_MESSAGE(sessionStarted)
  FORBOCAI_READ_TERMINAL_MESSAGE(stepStarted)
  FORBOCAI_READ_TERMINAL_MESSAGE(commandResult)
  FORBOCAI_READ_TERMINAL_MESSAGE(transcriptHeading)
  FORBOCAI_READ_TERMINAL_MESSAGE(invalidMode)
#undef FORBOCAI_READ_TERMINAL_MESSAGE
  Data.usage = DataAdapters::ReadStringArrayField(Source.Root, TEXT("usage"));
#define FORBOCAI_READ_TERMINAL_GRID(Name)                                 \
  Data.grid.Name = DataAdapters::ReadStringField(Grid, TEXT(#Name));
  FORBOCAI_READ_TERMINAL_GRID(blocked)
  FORBOCAI_READ_TERMINAL_GRID(player)
  FORBOCAI_READ_TERMINAL_GRID(open)
  FORBOCAI_READ_TERMINAL_GRID(cellSeparator)
  FORBOCAI_READ_TERMINAL_GRID(rowSeparator)
#undef FORBOCAI_READ_TERMINAL_GRID
  Data.npcGlyphs = detail::ReadNpcGlyphs(
      DataAdapters::ReadObjectArrayField(Source.Root, TEXT("npcGlyphs")));
  Data.unknownNpcGlyph =
      DataAdapters::ReadStringField(Source.Root, TEXT("unknownNpcGlyph"));
  Data.levels.display =
      DataAdapters::ReadBooleanField(Levels, TEXT("display"));
  Data.levels.error =
      DataAdapters::ReadBooleanField(Levels, TEXT("error"));
  Data.transcript.idFormat =
      DataAdapters::ReadStringField(Transcript, TEXT("idFormat"));
  Data.conversationTranscript.ErrorStatus = DataAdapters::ReadStringField(
      ConversationTranscript, TEXT("errorStatus"));
  Data.conversationTranscript.FailureFallback =
      DataAdapters::ReadStringField(ConversationTranscript,
                                    TEXT("failureFallback"));
  Data.conversationTranscript.TranscriptLinesField =
      DataAdapters::ReadStringField(ConversationTranscript,
                                    TEXT("transcriptLinesField"));
  Data.conversationTranscript.LineTextField =
      DataAdapters::ReadStringField(ConversationTranscript,
                                    TEXT("lineTextField"));
  Data.conversationTranscript.FailureLines = detail::ReadAuthoredLines(
      DataAdapters::ReadObjectArrayField(ConversationTranscript,
                                         TEXT("failureLines")));
  Data.tokens.name = DataAdapters::ReadStringField(Tokens, TEXT("name"));
  Data.tokens.x = DataAdapters::ReadStringField(Tokens, TEXT("x"));
  Data.tokens.y = DataAdapters::ReadStringField(Tokens, TEXT("y"));
#define FORBOCAI_READ_TERMINAL_TOKEN(Name)                                \
  Data.tokens.Name = DataAdapters::ReadStringField(Tokens, TEXT(#Name));
  FORBOCAI_READ_TERMINAL_TOKEN(timestamp)
  FORBOCAI_READ_TERMINAL_TOKEN(status)
  FORBOCAI_READ_TERMINAL_TOKEN(command)
  FORBOCAI_READ_TERMINAL_TOKEN(mode)
  FORBOCAI_READ_TERMINAL_TOKEN(title)
  FORBOCAI_READ_TERMINAL_TOKEN(id)
  FORBOCAI_READ_TERMINAL_TOKEN(output)
#undef FORBOCAI_READ_TERMINAL_TOKEN
  return Data;
}

/** User Story: As a terminal consumer, I need one immutable terminal-data instance so root-store initialization and listeners share the same authored configuration. @fn inline const FTerminalData &TerminalData() */
inline const FTerminalData &TerminalData() {
  static const FTerminalData Data = ReadTerminalData();
  return Data;
}

} // namespace MicroGame::TerminalAdapters
