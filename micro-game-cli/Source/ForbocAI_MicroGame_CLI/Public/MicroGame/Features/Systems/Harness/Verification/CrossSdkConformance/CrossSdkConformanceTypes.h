#pragma once

#include "MicroGame/Features/Components/Spatial/Grid/GridTypes.h"
#include "MicroGame/Features/Entities/NPCs/NPCsTypes.h"
#include "MicroGame/Features/Systems/Bridge/BridgeTypes.h"
#include "MicroGame/Features/Systems/Memory/MemoryTypes.h"
#include "MicroGame/Features/Systems/Social/SocialTypes.h"

namespace MicroGame::CrossSdkConformance {

struct FBridgeGridStories {
  FString jump;
  FString move;
  FString grid;
  FString coverage;
};

struct FBridgeGridJumpData {
  int32 requestedForce{};
  FString reasonFragment;
};

struct FBridgeGridMoveData {
  int32 requestedDistance{};
  int32 expectedDistance{};
};

struct FBridgeGridData {
  FGridState state;
  FPosition blockedPosition;
  FPosition openPosition;
  FPosition outsidePosition;
};

struct FBridgeGridCoverageData {
  TMap<FString, bool> covered;
  int32 minimumMissing{};
  FString expectedMissing;
};

struct FBridgeGridVerificationData {
  FString suite;
  FString automationName;
  FBridgeGridStories stories;
  FBridgeRulesState rules;
  FBridgeGridJumpData jump;
  FBridgeGridMoveData move;
  FBridgeGridData grid;
  FBridgeGridCoverageData coverage;
};

struct FCoverageVerificationData {
  FString suite;
  FString story;
};

struct FRtkStories {
  FString player;
  FString bridge;
};

struct FRtkAutomationNames {
  FString player;
  FString bridge;
};

struct FRtkPlayerPatchData {
  int32 hp{};
  bool hidden{};
  FPosition position;
  TArray<FString> inventory;
};

struct FRtkPlayerData {
  FRtkPlayerPatchData patch;
  FString expectedName;
};

struct FRtkBridgePatchData {
  int32 maxJumpForce{};
  FString activePreset;
};

struct FRtkBridgePresetData {
  FString name;
  int32 expectedDistance{};
};

struct FRtkBridgePresetsData {
  FRtkBridgePresetData social;
  FRtkBridgePresetData custom;
};

struct FRtkBridgeData {
  FRtkBridgePatchData patch;
  int32 expectedInitialDistance{};
  FRtkBridgePresetsData presets;
};

struct FRtkVerificationData {
  FString suite;
  FRtkAutomationNames automationNames;
  FRtkStories stories;
  FRtkPlayerData player;
  FRtkBridgeData bridge;
};

struct FNpcPatchData {
  FString name;
  int32 hp{};
  int32 suspicion{};
  TArray<FString> inventory;
  FPosition position;
};

struct FNpcVerdictData {
  FString actionType;
  FPosition targetPosition;
  int32 suspicion{};
};

struct FNpcVerificationData {
  FGameNPC initial;
  FNpcPatchData patch;
  FNpcVerdictData verdict;
  int32 expectedEntityCount{};
};

struct FStoreMemoryData {
  TArray<FMemoryRecord> records;
  FString lookupId;
  FString npcId;
  int32 expectedTotal{};
  int32 expectedNpcCount{};
};

struct FStoreInventoryData {
  FString ownerId;
  TArray<FString> items;
};

struct FStoreSoulData {
  FString npcId;
  FString transactionId;
  int32 expectedImportedCount{};
};

struct FStoreVerificationData {
  FStoreMemoryData memory;
  FStoreInventoryData inventory;
  FStoreSoulData soul;
};

struct FStateDomainStories {
  FString npc;
  FString store;
};

struct FStateDomainAutomationNames {
  FString npc;
  FString store;
};

struct FStateDomainsVerificationData {
  FString suite;
  FStateDomainAutomationNames automationNames;
  FStateDomainStories stories;
  FNpcVerificationData npc;
  FStoreVerificationData store;
};

struct FMechanicsStealthData {
  bool doorOpen{};
  int32 alertDelta{};
};

struct FMechanicsSocialData {
  FString dialogue;
  FTradeOffer tradeOffer;
};

struct FMechanicsUiData {
  FString message;
  int32 expectedMessageCount{};
};

struct FMechanicsTranscriptData {
  FString scenarioId;
  FString statusCommand;
  FString npcCommand;
  int32 expectedEntryCount{};
  int32 expectedErrorCount{};
};

struct FMechanicsCoverageData {
  TArray<FString> requiredGroups;
  FString coveredGroup;
  int32 expectedCoveredCount{};
  int32 expectedMissingCount{};
};

struct FMechanicsScenarioData {
  int32 expectedStepCount{};
};

struct FMechanicsVerificationData {
  FString suite;
  FString automationName;
  FString story;
  FMechanicsStealthData stealth;
  FMechanicsSocialData social;
  FMechanicsUiData ui;
  FMechanicsTranscriptData transcript;
  FMechanicsCoverageData coverage;
  FMechanicsScenarioData scenario;
};

struct FCrossSdkConformanceData {
  FBridgeGridVerificationData bridgeGrid;
  FCoverageVerificationData coverage;
  FRtkVerificationData rtk;
  FStateDomainsVerificationData stateDomains;
  FMechanicsVerificationData mechanics;
};

} // namespace MicroGame::CrossSdkConformance
