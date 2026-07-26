#pragma once

#include "Core/rtk.hpp"
#include "MicroGame/Features/Components/Inventory/InventoryTypes.h"
#include "MicroGame/Features/Components/Spatial/Grid/GridTypes.h"
#include "MicroGame/Features/Entities/NPCs/NPCsTypes.h"
#include "MicroGame/Features/Entities/Player/PlayerTypes.h"
#include "MicroGame/Features/Systems/Bridge/BridgeTypes.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageTypes.h"
#include "MicroGame/Features/Systems/Harness/Verification/Vocabulary/VerificationVocabularyTypes.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"
#include "MicroGame/Features/Systems/Memory/MemoryTypes.h"
#include "MicroGame/Features/Systems/Quality/QualityTypes.h"
#include "MicroGame/Features/Systems/Social/SocialTypes.h"
#include "MicroGame/Features/Systems/Soul/SoulTypes.h"
#include "MicroGame/Features/Systems/Stealth/StealthTypes.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {

struct FMicroGameState {
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

typedef rtk::EnhancedStore<FMicroGameState> FMicroGameStore;
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

} // namespace MicroGame
