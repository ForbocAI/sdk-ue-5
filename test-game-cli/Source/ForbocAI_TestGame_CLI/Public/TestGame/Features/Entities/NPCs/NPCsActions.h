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

inline rtk::ActionCreator<FMoveNPCPayload> MoveNPCActionCreator() {
  static auto C =
      rtk::createAction<FMoveNPCPayload>(TEXT("testgame/npcs/moveNPC"));
  return C;
}

inline rtk::ActionCreator<FPatchNPCPayload> PatchNPCActionCreator() {
  static auto C =
      rtk::createAction<FPatchNPCPayload>(TEXT("testgame/npcs/patchNPC"));
  return C;
}

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
