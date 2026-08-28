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
  FString TextOption;
  FString LegalActionsOption;
  FString VisitedActionsOption;
  FString AvoidActionsOption;
  FString PersonaMemoryType;
  FString LegalActionsSeparator;
  FString MessageSeparator;
  FString PersonaPattern;
  FString FormatOpenToken;
  FString FormatCloseToken;
};

struct FCLINPCAnalysis {
  FString DiagnosisContainer;
  FString DiagnosticResultField;
  FString OutputLineSeparator;
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
  FString UpdateUsage;
  FString Updated;
  FString ProcessUsage;
  FString GenerateUsage;
  FString DecideUsage;
  FString RecallUsage;
  FString Recalled;
  FString DialogueFallback;
  FString Dialogue;
  FString Action;
  FString ThoughtResultPrefix;
  FString ReasoningResultPrefix;
  FString DiagnosticResultPrefix;
  FString PromptTraceEnvKey;
  FString PromptTraceHeader;
  FString PromptTraceFooter;
  FString ProfileOption;
  FString ThoughtProfileMemoryType;
  FString ThoughtProfileStateKey;
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
  FString EmptyPersona;
  FString ApiFailure;
  TArray<FString> NetworkErrors;
};

struct FCLINPCState {
  FCLINPCLimits Limits;
  FCLINPCSyntax Syntax;
  FCLINPCAnalysis Analysis;
  FCLINPCMessages Messages;
};

struct FCLINPCUpdate {
  func::Maybe<FString> RequestedNpcId;
  FAgentState Delta;
  TArray<FString> Fields;
};

struct FCLIOptionSpec {
  FString Option;
  int32 ValueOffset;
  FString OptionPrefix;
};

struct FCLIOptionExtraction {
  func::Maybe<FString> Value;
  TArray<FString> Rest;
};

struct FCLINPCDecision {
  FString NpcId;
  FString Observation;
  bool bHasProfile{};
  FString Profile;
  bool bHasLegalActions{};
  TArray<FString> LegalActions;
  bool bHasVisitedActions{};
  TArray<FString> VisitedActions;
  bool bHasAvoidActions{};
  TArray<FString> AvoidActions;
};

} // namespace NPC
} // namespace CLI
} // namespace ForbocAI
