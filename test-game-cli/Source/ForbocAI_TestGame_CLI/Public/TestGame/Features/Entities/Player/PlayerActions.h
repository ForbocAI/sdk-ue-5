#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"

namespace TestGame {

/** User Story: As a features entities player consumer, I need to invoke set position action creator through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::ActionCreator<FPosition> setPositionActionCreator() */
inline rtk::ActionCreator<FPosition> setPositionActionCreator() {
  static auto C =
      rtk::createAction<FPosition>(TEXT("testgame/player/setPosition"));
  return C;
}

/** User Story: As a features entities player consumer, I need to invoke set hidden action creator through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::ActionCreator<bool> setHiddenActionCreator() */
inline rtk::ActionCreator<bool> setHiddenActionCreator() {
  static auto C = rtk::createAction<bool>(TEXT("testgame/player/setHidden"));
  return C;
}

struct FPatchPlayerPayload {
  FString Name;
  bool bHasName;
  int32 Hp;
  bool bHasHp;
  bool bHidden;
  bool bHasHidden;
  FPosition Position;
  bool bHasPosition;
  TArray<FString> Inventory;
  bool bHasInventory;

  /** User Story: As a features entities player consumer, I need to invoke fpatch player payload through a stable signature so the features entities player workflow remains explicit and composable. @fn FPatchPlayerPayload() */
  FPatchPlayerPayload()
      : bHasName(false), Hp(0), bHasHp(false), bHidden(false),
        bHasHidden(false), bHasPosition(false), bHasInventory(false) {}
};

/** User Story: As a features entities player consumer, I need to invoke patch player action creator through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::ActionCreator<FPatchPlayerPayload> patchPlayerActionCreator() */
inline rtk::ActionCreator<FPatchPlayerPayload> patchPlayerActionCreator() {
  static auto C =
      rtk::createAction<FPatchPlayerPayload>(TEXT("testgame/player/patchPlayer"));
  return C;
}

namespace PlayerActions {

/** User Story: As a features entities player consumer, I need to invoke set position through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::AnyAction setPosition(const FPosition &P) */
inline rtk::AnyAction setPosition(const FPosition &P) {
  return setPositionActionCreator()(P);
}

/** User Story: As a features entities player consumer, I need to invoke set hidden through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::AnyAction setHidden(bool H) */
inline rtk::AnyAction setHidden(bool H) {
  return setHiddenActionCreator()(H);
}

/** User Story: As a features entities player consumer, I need to invoke patch player through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::AnyAction patchPlayer(const FPatchPlayerPayload &P) */
inline rtk::AnyAction patchPlayer(const FPatchPlayerPayload &P) {
  return patchPlayerActionCreator()(P);
}

} // namespace PlayerActions
} // namespace TestGame
