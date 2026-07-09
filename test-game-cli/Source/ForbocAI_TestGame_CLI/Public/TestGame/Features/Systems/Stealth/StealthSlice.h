#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Stealth/StealthActions.h"
#include "TestGame/Features/Systems/Stealth/StealthTypes.h"

namespace TestGame {

namespace StealthSelectors {
inline bool SelectStealthDoorOpen(const FStealthState &S) {
  return S.bDoorOpen;
}
inline int32 SelectStealthAlertLevel(const FStealthState &S) {
  return S.AlertLevel;
}
} // namespace StealthSelectors

inline rtk::Slice<FStealthState> CreateStealthSlice() {
  return rtk::createSlice<FStealthState>(
      TEXT("testgame/stealth"), FStealthState(),
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
                  FMath::Clamp(Next.AlertLevel + A.PayloadValue, 0, 100);
              return Next;
            });
      });
}

} // namespace TestGame
