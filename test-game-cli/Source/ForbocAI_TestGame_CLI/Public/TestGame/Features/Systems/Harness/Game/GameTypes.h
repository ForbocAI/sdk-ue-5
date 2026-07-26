#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"
#include "TestGame/Features/Components/Spatial/Grid/GridTypes.h"
#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"
#include "TestGame/Features/Systems/Harness/Game/Vocabulary/GameVocabularyTypes.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"
#include "TestGame/Features/Systems/Quality/QualityTypes.h"
#include "TestGame/Features/Systems/Social/SocialTypes.h"
#include "TestGame/Features/Systems/Soul/SoulTypes.h"
#include "TestGame/Features/Systems/Stealth/StealthTypes.h"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {

struct FTestGameState {
  FNPCsSliceState NPCs;
  FPlayerState Player;
  FGridState Grid;
  FStealthState Stealth;
  FSocialState Social;
  FBridgeRulesState Bridge;
  FGameMemorySliceState Memory;
  FInventoryState Inventory;
  FSoulTrackingState Soul;
  FUIState UI;
  FTranscriptState Transcript;
  FScenarioSliceState Scenario;
  FHarnessState Harness;
  CommandRunner::FCommandAliasState CommandRunner;
  FQualityState Quality;
};

typedef rtk::EnhancedStore<FTestGameState> FTestGameStore;
typedef rtk::Dispatcher FAppDispatch;

struct FGameRunResult {
  bool bComplete{};
  TArray<FString> MissingGroups;
  TArray<FTranscriptEntry> Transcript;
  int32 TranscriptErrorCount{};
  bool bQualityRequired{};
  bool bQualityGatePassed{};
  func::Maybe<FQualityReport> QualityReport;
  FString QualityReportPath;
  FString Summary;
};

struct FParsedVerdict {
  bool bValid{};
  FString ActionType;
  FPosition TargetHex;
  int32 SuspicionDelta{};
};

struct FGameProgress {
  FString Type;
  FString Mode;
  FScenarioStep Step;
  FCommandSpec Command;
  CommandRunner::FCommandOutput CommandResult;
  FGameRunResult RunResult;
  FString Message;
};

} // namespace TestGame
