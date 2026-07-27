#pragma once

#include "CoreMinimal.h"
#include "Components/NPC/Agent/AgentTypes.h"

namespace ForbocAI {
namespace CLI {
namespace NPC {

struct FCLINPCLimits {
  int32 EmptyArgumentCount;
  int32 SingleArgumentCount;
  int32 DoubleArgumentCount;
  int32 UpdateArgumentCount;
  int32 ActiveUpdateArgumentCount;
  int32 FirstArgumentIndex;
  int32 FirstUpdateIndex;
  int32 ActiveUpdateIndex;
  int32 PairStride;
  int32 KeyOffset;
  int32 ValueOffset;
};

struct FCLINPCSyntax {
  FString OptionPrefix;
  FString MessageSeparator;
  FString PersonaPattern;
};

struct FCLINPCMessages {
  FString CreateUsage;
  FString Creating;
  FString Created;
  FString Id;
  FString State;
  FString NoActive;
  FString NotFound;
  FString UpdateUsage;
  FString Updated;
  FString ProcessUsage;
  FString DialogueFallback;
  FString Dialogue;
  FString Action;
  FString ChatUsage;
  FString ChatMissingId;
  FString ChatHeader;
  FString ChatUser;
  FString ChatNpc;
  FString ChatAction;
  TArray<FString> ExitCommands;
  FString ImportUsage;
  FString Imported;
  FString Persona;
  FString ImportDone;
  FString StatePrinted;
  FString UpdateDone;
};

struct FCLINPCState {
  FCLINPCLimits Limits;
  FCLINPCSyntax Syntax;
  FCLINPCMessages Messages;
};

struct FCLINPCUpdate {
  FString NpcId;
  FAgentState Delta;
  TArray<FString> Fields;
};

} // namespace NPC
} // namespace CLI
} // namespace ForbocAI
