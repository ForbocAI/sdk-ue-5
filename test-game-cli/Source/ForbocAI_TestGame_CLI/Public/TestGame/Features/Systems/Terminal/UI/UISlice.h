#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

inline rtk::Slice<FUIState> CreateUISlice() {
  return rtk::createSlice<FUIState>(
      TEXT("testgame/ui"), FUIState(),
      [](rtk::ActionReducerMapBuilder<FUIState> &Builder) {
        Builder.addCase(
            UIActions::setModeActionCreator(),
            [](const FUIState &S,
               const rtk::Action<EPlayMode> &A) -> FUIState {
              FUIState Next = S;
              Next.Mode = A.PayloadValue;
              return Next;
            });
        Builder.addCase(
            UIActions::addMessageActionCreator(),
            [](const FUIState &S,
               const rtk::Action<FString> &A) -> FUIState {
              FUIState Next = S;
              Next.Messages.Add(A.PayloadValue);
              return Next;
            });
      });
}

} // namespace TestGame
