#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"

namespace TestGame {
namespace NPCsActions {

/** User Story: As a features entities npcs consumer, I need to invoke upsert npcaction creator through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::ActionCreator<FGameNPC> UpsertNPCActionCreator() */
inline rtk::ActionCreator<FGameNPC> UpsertNPCActionCreator() {
  static auto C = rtk::createAction<FGameNPC>(TEXT("testgame/npcs/upsertNPC"));
  return C;
}

/** User Story: As a features entities npcs consumer, I need to invoke move npcaction creator through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::ActionCreator<FMoveNPCPayload> MoveNPCActionCreator() */
inline rtk::ActionCreator<FMoveNPCPayload> MoveNPCActionCreator() {
  static auto C =
      rtk::createAction<FMoveNPCPayload>(TEXT("testgame/npcs/moveNPC"));
  return C;
}

/** User Story: As a features entities npcs consumer, I need to invoke patch npcaction creator through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::ActionCreator<FPatchNPCPayload> PatchNPCActionCreator() */
inline rtk::ActionCreator<FPatchNPCPayload> PatchNPCActionCreator() {
  static auto C =
      rtk::createAction<FPatchNPCPayload>(TEXT("testgame/npcs/patchNPC"));
  return C;
}

/** User Story: As a features entities npcs consumer, I need to invoke apply npc verdict action creator through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::ActionCreator<FApplyNpcVerdictPayload> ApplyNpcVerdictActionCreator() */
inline rtk::ActionCreator<FApplyNpcVerdictPayload>
ApplyNpcVerdictActionCreator() {
  static auto C = rtk::createAction<FApplyNpcVerdictPayload>(
      TEXT("testgame/npcs/applyNpcVerdict"));
  return C;
}

/** User Story: As a features entities npcs consumer, I need to invoke upsert npc through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::AnyAction UpsertNPC(const FGameNPC &N) */
inline rtk::AnyAction UpsertNPC(const FGameNPC &N) {
  return UpsertNPCActionCreator()(N);
}

/** User Story: As a features entities npcs consumer, I need to invoke move npc through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::AnyAction MoveNPC(const FMoveNPCPayload &P) */
inline rtk::AnyAction MoveNPC(const FMoveNPCPayload &P) {
  return MoveNPCActionCreator()(P);
}

/** User Story: As a features entities npcs consumer, I need to invoke patch npc through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::AnyAction PatchNPC(const FPatchNPCPayload &P) */
inline rtk::AnyAction PatchNPC(const FPatchNPCPayload &P) {
  return PatchNPCActionCreator()(P);
}

/** User Story: As a features entities npcs consumer, I need to invoke apply npc verdict through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::AnyAction ApplyNpcVerdict(const FApplyNpcVerdictPayload &P) */
inline rtk::AnyAction ApplyNpcVerdict(const FApplyNpcVerdictPayload &P) {
  return ApplyNpcVerdictActionCreator()(P);
}

} // namespace NPCsActions
} // namespace TestGame
