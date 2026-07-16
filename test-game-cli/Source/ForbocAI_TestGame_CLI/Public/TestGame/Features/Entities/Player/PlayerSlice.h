#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/Player/PlayerActions.h"
#include "TestGame/Features/Entities/Player/PlayerAdapters.h"

namespace TestGame {

namespace PlayerSelectors {
/** User Story: As a features entities player consumer, I need to invoke select player name through a stable signature so the features entities player workflow remains explicit and composable. @fn inline FString SelectPlayerName(const FPlayerState &S) */
inline FString SelectPlayerName(const FPlayerState &S) { return S.Name; }
/** User Story: As a features entities player consumer, I need to invoke select player hp through a stable signature so the features entities player workflow remains explicit and composable. @fn inline int32 SelectPlayerHp(const FPlayerState &S) */
inline int32 SelectPlayerHp(const FPlayerState &S) { return S.Hp; }
/** User Story: As a features entities player consumer, I need to invoke select player hidden through a stable signature so the features entities player workflow remains explicit and composable. @fn inline bool SelectPlayerHidden(const FPlayerState &S) */
inline bool SelectPlayerHidden(const FPlayerState &S) { return S.bHidden; }
/** User Story: As a features entities player consumer, I need to invoke select player position through a stable signature so the features entities player workflow remains explicit and composable. @fn inline FPosition SelectPlayerPosition(const FPlayerState &S) */
inline FPosition SelectPlayerPosition(const FPlayerState &S) {
  return S.Position;
}
/** User Story: As a features entities player consumer, I need to invoke select player inventory through a stable signature so the features entities player workflow remains explicit and composable. @fn inline TArray<FString> SelectPlayerInventory(const FPlayerState &S) */
inline TArray<FString> SelectPlayerInventory(const FPlayerState &S) {
  return S.Inventory;
}
} // namespace PlayerSelectors

/** User Story: As a features entities player consumer, I need to invoke create player slice through a stable signature so the features entities player workflow remains explicit and composable. @fn inline rtk::Slice<FPlayerState> CreatePlayerSlice() */
inline rtk::Slice<FPlayerState> CreatePlayerSlice() {
  return rtk::createSlice<FPlayerState>(
      TEXT("testgame/player"), CreatePlayerInitialState(),
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
