#pragma once

#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"
#include "TestGame/Features/Systems/Grid/GridTypes.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"
#include "TestGame/Features/Systems/Inventory/InventoryTypes.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"
#include "TestGame/Features/Systems/Scenario/ScenarioTypes.h"
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
};

typedef rtk::EnhancedStore<FTestGameState> FTestGameStore;
typedef rtk::Dispatcher FAppDispatch;

struct FGameRunResult {
  bool bComplete;
  TArray<ECommandGroup> MissingGroups;
  TArray<FTranscriptEntry> Transcript;
  FString Summary;

  FGameRunResult() : bComplete(false) {}
};

struct FParsedVerdict {
  bool bValid;
  FString ActionType;
  FPosition TargetHex;
  int32 SuspicionDelta;

  FParsedVerdict() : bValid(false), SuspicionDelta(0) {}
};

enum class EGameProgressType : uint8 {
  SessionStarted,
  ContractFailed,
  StepStarted,
  CommandCompleted,
  SessionCompleted
};

struct FGameProgress {
  EGameProgressType Type;
  EPlayMode Mode;
  FScenarioStep Step;
  FCommandSpec Command;
  CommandRunner::FCommandOutput CommandResult;
  FGameRunResult RunResult;
  FString Message;

  FGameProgress()
      : Type(EGameProgressType::SessionStarted),
        Mode(EPlayMode::Autoplay) {}
};

} // namespace TestGame
