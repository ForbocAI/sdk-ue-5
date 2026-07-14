#pragma once
/**
 * Presentation-only terminal view helpers.
 */

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"

namespace TestGame {

inline FString RenderGrid(const FTerminalRenderState &State) {
  return State.GridText;
}

inline FString RenderLegend() {
  return TEXT("Legend :: P=Scout  D=Doomguard  M=Miller  #=Blocked  .=Open");
}

namespace TerminalViewDetail {

inline void PresentDisplayLine(const FString &Line) {
  UE_LOG(LogTemp, Display, TEXT("%s"), *Line);
}

inline void PresentErrorLine(const FString &Line) {
  UE_LOG(LogTemp, Error, TEXT("%s"), *Line);
}

inline void PresentLines(const TArray<FTerminalLineViewModel> &Lines,
                         int32 Index) {
  Index >= Lines.Num()
      ? void()
      : (Lines[Index].Level == ETerminalLineLevel::Error
             ? PresentErrorLine(Lines[Index].Text)
             : PresentDisplayLine(Lines[Index].Text),
         PresentLines(Lines, Index + 1));
}

} // namespace TerminalViewDetail

inline void PresentProgress(const FTerminalProgressViewModel &ViewModel) {
  TerminalViewDetail::PresentLines(ViewModel.Lines, 0);
}

} // namespace TestGame
