#pragma once

#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"
#include "TestGame/Features/Components/Spatial/Grid/GridTypes.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"
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
};

typedef rtk::EnhancedStore<FTestGameState> FTestGameStore;
typedef rtk::Dispatcher FAppDispatch;

struct FGameRunResult {
  bool bComplete;
  TArray<ECommandGroup> MissingGroups;
  TArray<FTranscriptEntry> Transcript;
  FString Summary;

  /** User Story: As a systems harness game consumer, I need to invoke fgame run result through a stable signature so the systems harness game workflow remains explicit and composable. @fn FGameRunResult() */
  FGameRunResult() : bComplete(false) {}
};

struct FParsedVerdict {
  bool bValid;
  FString ActionType;
  FPosition TargetHex;
  int32 SuspicionDelta;

  /** User Story: As a systems harness game consumer, I need to invoke fparsed verdict through a stable signature so the systems harness game workflow remains explicit and composable. @fn FParsedVerdict() */
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

  /** User Story: As a systems harness game consumer, I need to invoke fgame progress through a stable signature so the systems harness game workflow remains explicit and composable. @fn FGameProgress() */
  FGameProgress()
      : Type(EGameProgressType::SessionStarted),
        Mode(EPlayMode::Autoplay) {}
};

} // namespace TestGame
