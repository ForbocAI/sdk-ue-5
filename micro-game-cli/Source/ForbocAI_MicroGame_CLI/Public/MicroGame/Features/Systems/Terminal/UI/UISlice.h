#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Terminal/UI/UIActions.h"

namespace MicroGame {

/** User Story: As a systems terminal ui consumer, I need to invoke create uislice through a stable signature so the systems terminal ui workflow remains explicit and composable. @fn inline rtk::Slice<FUIState> CreateUISlice() */
inline rtk::Slice<FUIState> CreateUISlice() {
  return rtk::createSlice<FUIState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV568BB8CE6D51), TerminalAdapters::TerminalData().initialState,
      [](rtk::ActionReducerMapBuilder<FUIState> &Builder) {
        Builder.addCase(
            UIActions::setModeActionCreator(),
            [](const FUIState &S,
               const rtk::Action<FString> &A) -> FUIState {
              return FUIState{A.PayloadValue, S.Messages};
            });
        Builder.addCase(
            UIActions::addMessageActionCreator(),
            [](const FUIState &S,
               const rtk::Action<FString> &A) -> FUIState {
              return FUIState{S.Mode, func::append_value<FString>(
                                         S.Messages, A.PayloadValue)};
            });
      });
}

} // namespace MicroGame
