#pragma once
/**
 * Presentation-only terminal view helpers.
 */

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"

namespace TestGame {

/** User Story: As a views terminal consumer, I need to invoke render grid through a stable signature so the views terminal workflow remains explicit and composable. @fn inline FString RenderGrid(const FTerminalRenderState &State) */
inline FString RenderGrid(const FTerminalRenderState &State) {
  return State.GridText;
}

/** User Story: As a views terminal consumer, I need to invoke render legend through a stable signature so the views terminal workflow remains explicit and composable. @fn inline FString RenderLegend() */
inline FString RenderLegend() {
  return TEXT("Legend :: P=Scout  D=Doomguard  M=Miller  #=Blocked  .=Open");
}

namespace TerminalViewDetail {

/** User Story: As a views terminal consumer, I need to invoke present display line through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentDisplayLine(const FString &Line) */
inline void PresentDisplayLine(const FString &Line) {
  UE_LOG(LogTemp, Display, TEXT("%s"), *Line);
}

/** User Story: As a views terminal consumer, I need to invoke present error line through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentErrorLine(const FString &Line) */
inline void PresentErrorLine(const FString &Line) {
  UE_LOG(LogTemp, Error, TEXT("%s"), *Line);
}

/** User Story: As a views terminal consumer, I need to invoke present lines through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentLines(const TArray<FTerminalLineViewModel> &Lines, int32 Index) */
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

/** User Story: As a views terminal consumer, I need to invoke present progress through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentProgress(const FTerminalProgressViewModel &ViewModel) */
inline void PresentProgress(const FTerminalProgressViewModel &ViewModel) {
  TerminalViewDetail::PresentLines(ViewModel.Lines, 0);
}

} // namespace TestGame
