#pragma once
/**
 * Presentation-only terminal view helpers.
 */

#include "Algo/ForEach.h"
#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"

namespace TestGame {

namespace TerminalViewDetail {

/** User Story: As a views terminal consumer, I need to invoke present display line through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentDisplayLine(const FString &Line) */
inline void PresentDisplayLine(const FString &Line) {
  GLog->Serialize(*Line, ELogVerbosity::Display, NAME_None);
}

/** User Story: As a views terminal consumer, I need to invoke present error line through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentErrorLine(const FString &Line) */
inline void PresentErrorLine(const FString &Line) {
  GLog->Serialize(*Line, ELogVerbosity::Error, NAME_None);
}

} // namespace TerminalViewDetail

/** User Story: As a views terminal consumer, I need to invoke present progress through a stable signature so the views terminal workflow remains explicit and composable. @fn inline void PresentProgress(const FTerminalProgressViewModel &ViewModel) */
inline void PresentProgress(const FTerminalProgressViewModel &ViewModel) {
  Algo::ForEach(ViewModel.Lines, [](const FTerminalLineViewModel &Line) {
    Line.bError ? TerminalViewDetail::PresentErrorLine(Line.Text)
                : TerminalViewDetail::PresentDisplayLine(Line.Text);
  });
}

} // namespace TestGame
