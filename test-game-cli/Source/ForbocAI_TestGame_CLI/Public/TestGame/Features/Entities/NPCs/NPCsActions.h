#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"

namespace TestGame {
namespace NPCsActions {

inline rtk::ActionCreator<FGameNPC> UpsertNPCActionCreator() {
  static auto C = rtk::createAction<FGameNPC>(TEXT("testgame/npcs/upsertNPC"));
  return C;
}

struct FMoveNPCPayload {
  FString Id;
  FPosition Position;
};

inline rtk::ActionCreator<FMoveNPCPayload> MoveNPCActionCreator() {
  static auto C =
      rtk::createAction<FMoveNPCPayload>(TEXT("testgame/npcs/moveNPC"));
  return C;
}

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

  FPatchNPCChanges()
      : bHasName(false), bHasFaction(false), Hp(0), bHasHp(false),
        Suspicion(0), bHasSuspicion(false), bHasInventory(false),
        bHasKnownSecrets(false), bHasPosition(false) {}
};

struct FPatchNPCPayload {
  FString Id;
  FPatchNPCChanges Patch;
};

inline rtk::ActionCreator<FPatchNPCPayload> PatchNPCActionCreator() {
  static auto C =
      rtk::createAction<FPatchNPCPayload>(TEXT("testgame/npcs/patchNPC"));
  return C;
}

struct FNpcVerdictAction {
  FString Type;
  FPosition TargetHex;
  bool bHasTargetHex;

  FNpcVerdictAction() : bHasTargetHex(false) {}
};

struct FApplyNpcVerdictPayload {
  FString Id;
  FNpcVerdictAction Action;
  FPatchNPCChanges StateDelta;
};

inline rtk::ActionCreator<FApplyNpcVerdictPayload>
ApplyNpcVerdictActionCreator() {
  static auto C = rtk::createAction<FApplyNpcVerdictPayload>(
      TEXT("testgame/npcs/applyNpcVerdict"));
  return C;
}

inline rtk::AnyAction UpsertNPC(const FGameNPC &N) {
  return UpsertNPCActionCreator()(N);
}

inline rtk::AnyAction MoveNPC(const FMoveNPCPayload &P) {
  return MoveNPCActionCreator()(P);
}

inline rtk::AnyAction PatchNPC(const FPatchNPCPayload &P) {
  return PatchNPCActionCreator()(P);
}

inline rtk::AnyAction ApplyNpcVerdict(const FApplyNpcVerdictPayload &P) {
  return ApplyNpcVerdictActionCreator()(P);
}

} // namespace NPCsActions
} // namespace TestGame
