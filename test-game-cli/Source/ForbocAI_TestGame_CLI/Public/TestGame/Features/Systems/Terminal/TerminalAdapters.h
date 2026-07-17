#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame::TerminalAdapters {

namespace detail {

/** User Story: As a terminal presenter, I need NPC glyph entries decoded recursively so adding an entity marker requires authored data rather than selector code. @fn inline TArray<FTerminalNpcGlyph> ReadNpcGlyphs( const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index, TArray<FTerminalNpcGlyph> Acc) */
inline TArray<FTerminalNpcGlyph> ReadNpcGlyphs(
    const TArray<TSharedPtr<FJsonValue>> &Values, int32 Index,
    TArray<FTerminalNpcGlyph> Acc) {
  return Index >= Values.Num()
             ? Acc
             : [&]() {
                 const TSharedRef<FJsonObject> Object =
                     Values[Index]->AsObject().ToSharedRef();
                 Acc.Add(FTerminalNpcGlyph{
                     DataAdapters::ReadStringField(Object, TEXT("id")),
                     DataAdapters::ReadStringField(Object, TEXT("glyph"))});
                 return ReadNpcGlyphs(
                     Values,
                     Index + GameAdapters::GameRuntimeData()
                                 .numbers.nextIndex,
                     MoveTemp(Acc));
               }();
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
      DataAdapters::ReadObjectArrayField(Source.Root, TEXT("npcGlyphs")),
      GameAdapters::GameRuntimeData().numbers.emptyCount, {});
  Data.unknownNpcGlyph =
      DataAdapters::ReadStringField(Source.Root, TEXT("unknownNpcGlyph"));
  Data.levels.display =
      DataAdapters::ReadBooleanField(Levels, TEXT("display"));
  Data.levels.error =
      DataAdapters::ReadBooleanField(Levels, TEXT("error"));
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
#undef FORBOCAI_READ_TERMINAL_TOKEN
  return Data;
}

/** User Story: As a terminal consumer, I need one immutable terminal-data instance so root-store initialization and listeners share the same authored configuration. @fn inline const FTerminalData &TerminalData() */
inline const FTerminalData &TerminalData() {
  static const FTerminalData Data = ReadTerminalData();
  return Data;
}

} // namespace TestGame::TerminalAdapters
