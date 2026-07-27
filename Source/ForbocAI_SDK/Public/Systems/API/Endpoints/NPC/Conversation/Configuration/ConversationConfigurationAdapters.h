#pragma once

#include "Components/API/Endpoints/NPC/Conversation/Configuration/ConversationConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace APISlice::Endpoints::NPCConversationConfiguration {

/** User Story: As an NPC conversation codec, I need field names and limits decoded from authored data. @fn inline FConversationConfigurationData readConversationConfigurationData() */
inline FConversationConfigurationData readConversationConfigurationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/npc-conversation.json"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Errors =
      DataAdapters::ReadObjectField(Source, TEXT("errors"));
  FConversationConfigurationData Data;
#define FORBOCAI_READ_CONVERSATION_FIELD(Name, JsonName)                  \
  Data.Fields.Name = DataAdapters::ReadStringField(                       \
      Fields, TEXT(#JsonName));
  FORBOCAI_NPC_CONVERSATION_FIELD_NAMES(
      FORBOCAI_READ_CONVERSATION_FIELD)
#undef FORBOCAI_READ_CONVERSATION_FIELD
  Data.Limits.ParticipantCount = DataAdapters::ReadNumberField(
      Limits, TEXT("participantCount"));
  Data.Limits.MinimumTurnCount = DataAdapters::ReadNumberField(
      Limits, TEXT("minimumTurnCount"));
  Data.Limits.MinimumTurnIndex = DataAdapters::ReadNumberField(
      Limits, TEXT("minimumTurnIndex"));
  Data.Limits.MinimumTextLength = DataAdapters::ReadNumberField(
      Limits, TEXT("minimumTextLength"));
  Data.Limits.MinimumStringArrayCount = DataAdapters::ReadNumberField(
      Limits, TEXT("minimumStringArrayCount"));
  Data.Limits.MinimumTranscriptLineCount = DataAdapters::ReadNumberField(
      Limits, TEXT("minimumTranscriptLineCount"));
  Data.Errors = {
      DataAdapters::ReadStringField(Errors, TEXT("invalidResponse")),
      DataAdapters::ReadStringField(Errors, TEXT("invalidParticipant")),
      DataAdapters::ReadStringField(Errors, TEXT("invalidTurn")),
      DataAdapters::ReadStringField(Errors, TEXT("invalidParticipantCount")),
      DataAdapters::ReadStringField(Errors, TEXT("invalidTurnCount"))};
  return Data;
}

/** User Story: As an NPC conversation codec, I need one immutable package configuration instance. @fn inline const FConversationConfigurationData &conversationConfigurationData() */
inline const FConversationConfigurationData &conversationConfigurationData() {
  static const FConversationConfigurationData Data =
      readConversationConfigurationData();
  return Data;
}

} // namespace APISlice::Endpoints::NPCConversationConfiguration
