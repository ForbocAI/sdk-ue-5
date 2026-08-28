#pragma once

#include "Components/NPC/Agent/AgentTypes.h"
#include "Components/NPC/NPCTypes.h"
#include "Core/fp.hpp"

struct FActorUpdateInput {
  func::Maybe<FString> RequestedActorId;
  FAgentState Delta;
};

struct FActorUpdateResult {
  func::Maybe<FString> TargetActorId;
  func::Maybe<FNPCInternalState> Actor;
};
