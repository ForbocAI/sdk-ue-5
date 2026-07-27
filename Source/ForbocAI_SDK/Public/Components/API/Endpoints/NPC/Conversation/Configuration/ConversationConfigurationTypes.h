#pragma once

#include "CoreMinimal.h"

#define FORBOCAI_NPC_CONVERSATION_FIELD_NAMES(X)                         \
  X(SchemaVersion, schemaVersion)                                         \
  X(ConversationId, conversationId)                                       \
  X(Valid, valid)                                                         \
  X(Participants, participants)                                           \
  X(Topic, topic)                                                         \
  X(Opening, opening)                                                     \
  X(Turns, turns)                                                         \
  X(TranscriptLines, transcriptLines)                                     \
  X(Slot, slot)                                                           \
  X(NpcId, npcId)                                                         \
  X(Name, name)                                                           \
  X(Role, role)                                                           \
  X(Traits, traits)                                                       \
  X(Goals, goals)                                                         \
  X(Relationships, relationships)                                         \
  X(Location, location)                                                   \
  X(Mood, mood)                                                           \
  X(SpeakingStyle, speakingStyle)                                         \
  X(Constraints, constraints)                                             \
  X(Index, index)                                                         \
  X(SpeakerSlot, speakerSlot)                                             \
  X(SpeakerName, speakerName)                                             \
  X(ListenerSlot, listenerSlot)                                           \
  X(ListenerName, listenerName)                                           \
  X(Dialogue, dialogue)                                                   \
  X(Action, action)

namespace APISlice::Endpoints::NPCConversationConfiguration {

struct FConversationFields {
#define FORBOCAI_DECLARE_CONVERSATION_FIELD(Name, JsonName) FString Name;
  FORBOCAI_NPC_CONVERSATION_FIELD_NAMES(
      FORBOCAI_DECLARE_CONVERSATION_FIELD)
#undef FORBOCAI_DECLARE_CONVERSATION_FIELD
};

struct FConversationLimits {
  int32 ParticipantCount{};
  int32 MinimumTurnCount{};
  int32 MinimumTurnIndex{};
  int32 MinimumTextLength{};
  int32 MinimumStringArrayCount{};
  int32 MinimumTranscriptLineCount{};
};

struct FConversationErrors {
  FString InvalidResponse;
  FString InvalidParticipant;
  FString InvalidTurn;
  FString InvalidParticipantCount;
  FString InvalidTurnCount;
};

struct FConversationConfigurationData {
  FConversationFields Fields;
  FConversationLimits Limits;
  FConversationErrors Errors;
};

} // namespace APISlice::Endpoints::NPCConversationConfiguration
