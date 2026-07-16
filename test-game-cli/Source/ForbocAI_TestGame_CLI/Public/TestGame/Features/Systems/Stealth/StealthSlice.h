#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Stealth/StealthActions.h"
#include "TestGame/Features/Systems/Stealth/StealthAdapters.h"
#include "TestGame/Features/Systems/Stealth/StealthTypes.h"

namespace TestGame {

namespace StealthSelectors {
/** User Story: As a features systems stealth consumer, I need to invoke select stealth door open through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline bool SelectStealthDoorOpen(const FStealthState &S) */
inline bool SelectStealthDoorOpen(const FStealthState &S) {
  return S.bDoorOpen;
}
/** User Story: As a features systems stealth consumer, I need to invoke select stealth alert level through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline int32 SelectStealthAlertLevel(const FStealthState &S) */
inline int32 SelectStealthAlertLevel(const FStealthState &S) {
  return S.AlertLevel;
}
} // namespace StealthSelectors

/** User Story: As a features systems stealth consumer, I need to invoke create stealth slice through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline rtk::Slice<FStealthState> CreateStealthSlice() */
inline rtk::Slice<FStealthState> CreateStealthSlice() {
  return rtk::createSlice<FStealthState>(
      TEXT("testgame/stealth"), CreateStealthInitialState(),
      [](rtk::ActionReducerMapBuilder<FStealthState> &Builder) {
        Builder.addCase(
            StealthActions::setDoorOpenActionCreator(),
            [](const FStealthState &S,
               const rtk::Action<bool> &A) -> FStealthState {
              FStealthState Next = S;
              Next.bDoorOpen = A.PayloadValue;
              return Next;
            });
        Builder.addCase(
            StealthActions::bumpAlertActionCreator(),
            [](const FStealthState &S,
               const rtk::Action<int32> &A) -> FStealthState {
              FStealthState Next = S;
              Next.AlertLevel =
                  ClampStealthAlertLevel(Next.AlertLevel + A.PayloadValue);
              return Next;
            });
      });
}

} // namespace TestGame
