#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Chat/ChatTypes.h"

namespace MicroGame::ChatAdapters {

namespace detail {

/** User Story: As a chat presenter, I need authored template tokens replaced recursively so output text remains outside selector behavior. @fn inline FString FormatTemplateKeys(const FString &Template, const TMap<FString, FString> &Values, const TArray<FString> &Keys, int32 Index, int32 NextIndex) */
inline FString FormatTemplateKeys(const FString &Template,
                                  const TMap<FString, FString> &Values,
                                  const TArray<FString> &Keys, int32 Index,
                                  int32 NextIndex) {
  return Index >= Keys.Num()
             ? Template
             : FormatTemplateKeys(
                   Template.Replace(*Keys[Index], *Values.FindChecked(Keys[Index])),
                   Values, Keys, Index + NextIndex, NextIndex);
}

} // namespace detail

/** User Story: As a chat-log consumer, I need the shared transcript contract decoded once from authored data. @fn inline FChatData ReadChatData() */
inline FChatData ReadChatData() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/chat.json"));
  const TSharedRef<FJsonObject> Command =
      DataAdapters::ReadObjectField(Source, TEXT("command"));
  const TSharedRef<FJsonObject> Numbers =
      DataAdapters::ReadObjectField(Source, TEXT("numbers"));
  const TSharedRef<FJsonObject> Output =
      DataAdapters::ReadObjectField(Source, TEXT("output"));
  const TSharedRef<FJsonObject> Tokens =
      DataAdapters::ReadObjectField(Source, TEXT("tokens"));
  return FChatData{
      DataAdapters::ReadStringField(Source.Root, TEXT("sessionId")),
      DataAdapters::ReadStringArrayField(Source.Root, TEXT("probeIds")),
      {DataAdapters::ReadStringField(Command, TEXT("textOption"))},
      {DataAdapters::ReadNumberField(Numbers, TEXT("emptyCount")),
       DataAdapters::ReadNumberField(Numbers, TEXT("initialIndex")),
       DataAdapters::ReadNumberField(Numbers, TEXT("nextIndex"))},
      {DataAdapters::ReadStringField(Output, TEXT("empty")),
       DataAdapters::ReadStringField(Output, TEXT("heading")),
       DataAdapters::ReadStringField(Output, TEXT("user")),
       DataAdapters::ReadStringField(Output, TEXT("npc")),
       DataAdapters::ReadStringField(Output, TEXT("latency")),
       DataAdapters::ReadStringField(Output, TEXT("missing")),
       DataAdapters::ReadStringField(Output, TEXT("footer"))},
      {DataAdapters::ReadStringField(Tokens, TEXT("id")),
       DataAdapters::ReadStringField(Tokens, TEXT("prompt")),
       DataAdapters::ReadStringField(Tokens, TEXT("response")),
       DataAdapters::ReadStringField(Tokens, TEXT("durationMs"))}};
}

/** User Story: As a micro-game package, I need one immutable chat contract shared by selectors and tests. @fn inline const FChatData &ChatData() */
inline const FChatData &ChatData() {
  static const FChatData Data = ReadChatData();
  return Data;
}

/** User Story: As a chat presenter, I need exact authored tokens populated without embedding output prose in code. @fn inline FString FormatChatTemplate(const FString &Template, const TMap<FString, FString> &Values) */
inline FString FormatChatTemplate(const FString &Template,
                                  const TMap<FString, FString> &Values) {
  TArray<FString> Keys;
  Values.GetKeys(Keys);
  return detail::FormatTemplateKeys(
      Template, Values, Keys, ChatData().Numbers.InitialIndex,
      ChatData().Numbers.NextIndex);
}

} // namespace MicroGame::ChatAdapters
