#pragma once

#include "CoreMinimal.h"
#include "Components/CLI/Bridge/CLIBridgeTypes.h"
#include "Components/CLI/Ghost/CLIGhostTypes.h"
#include "Components/CLI/Memory/CLIMemoryTypes.h"
#include "Components/CLI/NPC/CLINPCTypes.h"
#include "Components/CLI/Soul/CLISoulTypes.h"

namespace ForbocAI {
namespace CLI {

struct FCLICommandSpec {
  FString Key;
  FString Group;
  FString Subcommand;
  TArray<FString> Surfaces;
  FString BrowserBehavior;
};

struct FCLIParsingSettings {
  int32 FirstTokenIndex;
  int32 SecondTokenIndex;
  int32 NextIndexOffset;
  int32 DirectDropCount;
  int32 SubcommandDropCount;
  int32 MinimumTokenCount;
};

struct FCLIInvocationSettings {
  TArray<FString> ApiUrlOptions;
  TArray<FString> ApiKeyOptions;
  FString AssignmentSeparator;
  int32 OptionValueOffset;
};

struct FCLICommandRoles {
  FString Version;
  FString Doctor;
  FString Status;
  FString Contract;
  FString Setup;
  FString SetupCheck;
  FString ConfigSet;
  FString ConfigGet;
  FString ConfigList;
  FString NpcCreate;
  FString NpcState;
  FString NpcUpdate;
  FString NpcProcess;
  FString NpcGenerate;
  FString NpcRecall;
  FString NpcDecide;
  FString NpcChat;
  FString NpcImport;
  FString SoulExport;
  FString SoulImport;
  FString SoulList;
  FString SoulChat;
  FString SoulVerify;
  FString RulesList;
  FString RulesPresets;
  FString GhostRun;
  FString GhostGenerate;
  FString GhostCreate;
  FString GhostUpdate;
  FString GhostRecall;
  FString GhostDecide;
  FString GhostMemoryStore;
  FString GhostStatus;
  FString GhostResults;
  FString GhostStop;
  FString GhostHistory;
  FString MemoryList;
  FString MemoryRecall;
  FString MemoryStore;
  FString MemoryClear;
  FString MemoryExport;
  FString BridgeValidate;
  FString BridgeRules;
  FString BridgePreset;
};

struct FCLIState {
  TArray<FCLICommandSpec> Commands;
  FString NodeSurface;
  FString BrowserSurface;
  FCLIParsingSettings Parsing;
  FCLIInvocationSettings Invocation;
  FCLICommandRoles CommandRoles;
  Bridge::FCLIBridgeState Bridge;
  Ghost::FCLIGhostState Ghost;
  Memory::FCLIMemoryState Memory;
  NPC::FCLINPCState NPC;
  Soul::FCLISoulState Soul;
};

struct FCommandParseResult {
  bool bMatched;
  FString CommandKey;
  TArray<FString> Args;
};

} // namespace CLI
} // namespace ForbocAI
