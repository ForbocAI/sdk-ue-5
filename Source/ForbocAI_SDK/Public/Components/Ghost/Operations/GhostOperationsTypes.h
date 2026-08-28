#pragma once

#include "Components/Actor/Operations/ActorOperationsTypes.h"
#include "Components/NPC/Agent/AgentTypes.h"
#include "Components/Protocol/Process/ProtocolProcessTypes.h"

struct FGhostAttributeGenerateInput {
  FString SessionId;
  FString Attribute;
  FString Context;
};

struct FGhostActorCreateInput {
  FString SessionId;
  FString Persona;
};

struct FGhostActorUpdateInput {
  FString SessionId;
  FActorUpdateInput Update;
};

struct FGhostActorRecallInput {
  FString SessionId;
  FString ActorId;
};

struct FGhostMemoryStoreInput {
  FString SessionId;
  FString ActorId;
  FString Observation;
  FString Type;
};
