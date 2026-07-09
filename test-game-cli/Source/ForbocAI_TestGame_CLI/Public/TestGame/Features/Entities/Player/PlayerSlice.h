#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/Player/PlayerActions.h"

namespace TestGame {

namespace PlayerSelectors {
inline FString SelectPlayerName(const FPlayerState &S) { return S.Name; }
inline int32 SelectPlayerHp(const FPlayerState &S) { return S.Hp; }
inline bool SelectPlayerHidden(const FPlayerState &S) { return S.bHidden; }
inline FPosition SelectPlayerPosition(const FPlayerState &S) {
  return S.Position;
}
inline TArray<FString> SelectPlayerInventory(const FPlayerState &S) {
  return S.Inventory;
}
} // namespace PlayerSelectors

inline rtk::Slice<FPlayerState> CreatePlayerSlice() {
  return rtk::createSlice<FPlayerState>(
      TEXT("testgame/player"), FPlayerState(),
      [](rtk::ActionReducerMapBuilder<FPlayerState> &Builder) {
        Builder.addCase(
            setPositionActionCreator(),
            [](const FPlayerState &S,
               const rtk::Action<FPosition> &A) -> FPlayerState {
              FPlayerState Next = S;
              Next.Position = A.PayloadValue;
              return Next;
            });
        Builder.addCase(
            setHiddenActionCreator(),
            [](const FPlayerState &S,
               const rtk::Action<bool> &A) -> FPlayerState {
              FPlayerState Next = S;
              Next.bHidden = A.PayloadValue;
              return Next;
            });
        Builder.addCase(
            patchPlayerActionCreator(),
            [](const FPlayerState &S,
               const rtk::Action<FPatchPlayerPayload> &A) -> FPlayerState {
              FPlayerState Next = S;
              Next.Name = A.PayloadValue.bHasName ? A.PayloadValue.Name
                                                  : Next.Name;
              Next.Hp = A.PayloadValue.bHasHp ? A.PayloadValue.Hp : Next.Hp;
              Next.bHidden = A.PayloadValue.bHasHidden
                                  ? A.PayloadValue.bHidden
                                  : Next.bHidden;
              Next.Position = A.PayloadValue.bHasPosition
                                  ? A.PayloadValue.Position
                                  : Next.Position;
              Next.Inventory = A.PayloadValue.bHasInventory
                                   ? A.PayloadValue.Inventory
                                   : Next.Inventory;
              return Next;
            });
      });
}

} // namespace TestGame
