#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Spatial/SpatialTypes.h"

namespace TestGame {

struct FGameNPC {
  FString Id;
  FString Name;
  FString Faction;
  int32 Hp;
  int32 Suspicion;
  TArray<FString> Inventory;
  TArray<FString> KnownSecrets;
  FPosition Position;

  /** User Story: As a features entities npcs consumer, I need to invoke fgame npc through a stable signature so the features entities npcs workflow remains explicit and composable. @fn FGameNPC() */
  FGameNPC() : Hp(100), Suspicion(0) {}
};

typedef FGameNPC FNPC;

struct FNPCsSliceState {
  rtk::EntityState<FGameNPC> Entities;

  /** User Story: As a features entities npcs consumer, I need to compare values for equality through a stable signature so the features entities npcs workflow remains explicit and composable. @fn bool operator==(const FNPCsSliceState &Other) const */
  bool operator==(const FNPCsSliceState &Other) const {
    return Entities.ids == Other.Entities.ids;
  }
};

namespace NPCsActions {

struct FMoveNPCPayload {
  FString Id;
  FPosition Position;
};

struct FPatchNPCChanges {
  FString Name;
  bool bHasName;
  FString Faction;
  bool bHasFaction;
  int32 Hp;
  bool bHasHp;
  int32 Suspicion;
  bool bHasSuspicion;
  TArray<FString> Inventory;
  bool bHasInventory;
  TArray<FString> KnownSecrets;
  bool bHasKnownSecrets;
  FPosition Position;
  bool bHasPosition;

  /** User Story: As a features entities npcs consumer, I need to invoke fpatch npcchanges through a stable signature so the features entities npcs workflow remains explicit and composable. @fn FPatchNPCChanges() */
  FPatchNPCChanges()
      : bHasName(false), bHasFaction(false), Hp(0), bHasHp(false),
        Suspicion(0), bHasSuspicion(false), bHasInventory(false),
        bHasKnownSecrets(false), bHasPosition(false) {}
};

struct FPatchNPCPayload {
  FString Id;
  FPatchNPCChanges Patch;
};

struct FNpcVerdictAction {
  FString Type;
  FPosition TargetHex;
  bool bHasTargetHex;

  /** User Story: As a features entities npcs consumer, I need to invoke fnpc verdict action through a stable signature so the features entities npcs workflow remains explicit and composable. @fn FNpcVerdictAction() */
  FNpcVerdictAction() : bHasTargetHex(false) {}
};

struct FApplyNpcVerdictPayload {
  FString Id;
  FNpcVerdictAction Action;
  FPatchNPCChanges StateDelta;
};

} // namespace NPCsActions

} // namespace TestGame
