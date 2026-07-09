#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"

namespace TestGame {

inline rtk::ActionCreator<FPosition> setPositionActionCreator() {
  static auto C =
      rtk::createAction<FPosition>(TEXT("testgame/player/setPosition"));
  return C;
}

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

  FPatchPlayerPayload()
      : bHasName(false), Hp(0), bHasHp(false), bHidden(false),
        bHasHidden(false), bHasPosition(false), bHasInventory(false) {}
};

inline rtk::ActionCreator<FPatchPlayerPayload> patchPlayerActionCreator() {
  static auto C =
      rtk::createAction<FPatchPlayerPayload>(TEXT("testgame/player/patchPlayer"));
  return C;
}

namespace PlayerActions {

inline rtk::AnyAction setPosition(const FPosition &P) {
  return setPositionActionCreator()(P);
}

inline rtk::AnyAction setHidden(bool H) {
  return setHiddenActionCreator()(H);
}

inline rtk::AnyAction patchPlayer(const FPatchPlayerPayload &P) {
  return patchPlayerActionCreator()(P);
}

} // namespace PlayerActions
} // namespace TestGame
