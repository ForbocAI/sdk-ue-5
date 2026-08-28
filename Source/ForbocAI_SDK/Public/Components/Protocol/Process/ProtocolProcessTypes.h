#pragma once

#include "Components/NPC/Agent/AgentTypes.h"

struct FProtocolProcessInput {
  FString NpcId;
  FString Observation;
  FString ContextJson;
  FString Persona;
  FAgentState InitialState;
  bool bHasThoughtProfile{};
  FString ThoughtProfile;
  bool bHasLegalActions{};
  TArray<FString> LegalActions;
  bool bHasVisitedActions{};
  TArray<FString> VisitedActions;
  bool bHasAvoidActions{};
  TArray<FString> AvoidActions;
};

struct FGhostProcessInput {
  FString SessionId;
  FProtocolProcessInput Process;
};
