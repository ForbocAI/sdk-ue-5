#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCBaseTypes.h"

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
  FString UnknownId;
  FString Created;
  FString Id;
  FString State;
  FString NoActive;
  FString MissingId;
  FString NotFound;
  FString ViewingActive;
  FString StateEntry;
  FString UpdateUsage;
  FString Updated;
  FString ProcessUsage;
  FString DialogueFallback;
  FString Dialogue;
  FString Action;
  FString Verdict;
  FString ProcessDone;
  FString ChatUsage;
  FString ChatMissingId;
  FString ChatHeader;
  FString ChatPrompt;
  FString ChatUser;
  FString ChatNpc;
  FString ChatAction;
  FString ChatDone;
  TArray<FString> ExitCommands;
  FString ImportUsage;
  FString Imported;
  FString ImportedLog;
  FString Persona;
  FString ImportDone;
  FString StatePrinted;
  FString UpdateDone;
  FString EmptyPersona;
  FString ApiFailure;
  TArray<FString> NetworkErrors;
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
