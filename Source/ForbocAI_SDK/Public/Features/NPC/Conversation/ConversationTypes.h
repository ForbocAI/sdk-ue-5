#pragma once

#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

struct FNPCConversationParticipant {
  FString Slot;
  FString NpcId;
  FString Name;
  FString Role;
  TArray<FString> Traits;
  TArray<FString> Goals;
  TArray<FString> Relationships;
  FString Location;
  FString Mood;
  TArray<FString> SpeakingStyle;
  TArray<FString> Constraints;
};

struct FNPCConversationTurn {
  int32 Index{};
  FString SpeakerSlot;
  FString SpeakerName;
  FString ListenerSlot;
  FString ListenerName;
  FString Dialogue;
  FAgentAction Action;
  bool bHasAction{};
  bool bValid{};
};

struct FNPCConversationResponse {
  FString SchemaVersion;
  FString ConversationId;
  bool bValid{};
  TArray<FNPCConversationParticipant> Participants;
  FString Topic;
  FString Opening;
  TArray<FNPCConversationTurn> Turns;
  TArray<FString> TranscriptLines;
  FString RawJson;
};
