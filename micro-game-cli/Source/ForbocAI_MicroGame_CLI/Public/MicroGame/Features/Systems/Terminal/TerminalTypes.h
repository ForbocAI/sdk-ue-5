#pragma once

#include "CoreMinimal.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace MicroGame {

struct FTranscriptEntry {
  FString Id;
  FString ScenarioId;
  FString CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  FString Status;
  FString Output;
  double DurationMs{};
  FString Timestamp;
};

struct FUIState {
  FString Mode;
  TArray<FString> Messages;
};

struct FTerminalMessages {
  FString npcMoved;
  FString legend;
  FString transcriptEntry;
  FString sessionStarted;
  FString stepStarted;
  FString commandResult;
  FString transcriptHeading;
  FString invalidMode;
};

struct FTerminalTokens {
  FString name;
  FString x;
  FString y;
  FString timestamp;
  FString status;
  FString command;
  FString mode;
  FString title;
  FString id;
  FString output;
};

struct FTerminalGridData {
  FString blocked;
  FString player;
  FString open;
  FString cellSeparator;
  FString rowSeparator;
};

struct FTerminalNpcGlyph {
  FString Id;
  FString Glyph;
};

struct FTerminalLevels {
  bool display{};
  bool error{};
};

struct FTerminalTranscriptData {
  FString idFormat;
};

struct FTerminalAuthoredLine {
  FString Template;
  bool bError{};
};

struct FTerminalConversationTranscriptData {
  FString ErrorStatus;
  FString FailureFallback;
  TArray<FTerminalAuthoredLine> FailureLines;
};

struct FTerminalData {
  FUIState initialState;
  FTerminalMessages messages;
  TArray<FString> usage;
  FTerminalGridData grid;
  TArray<FTerminalNpcGlyph> npcGlyphs;
  FString unknownNpcGlyph;
  FTerminalLevels levels;
  FTerminalTranscriptData transcript;
  FTerminalConversationTranscriptData conversationTranscript;
  FTerminalTokens tokens;
};

typedef FUIState FUiState;

struct FTranscriptState {
  TArray<FTranscriptEntry> Entries;
};

struct FTerminalRenderState {
  FString GridText;
  FString LegendText;
};

struct FTerminalLineViewModel {
  bool bError{};
  FString Text;

  /** User Story: As a terminal presenter, I need an empty line model so selectors can compose output without sentinel text. @fn FTerminalLineViewModel() = default */
  FTerminalLineViewModel() = default;

  /** User Story: As a terminal presenter, I need line severity and authored text carried together so views only perform output. @fn FTerminalLineViewModel(bool bInError, FString InText) */
  FTerminalLineViewModel(bool bInError, FString InText)
      : bError(bInError), Text(MoveTemp(InText)) {}
};

struct FTerminalProgressViewModel {
  TArray<FTerminalLineViewModel> Lines;
};

} // namespace MicroGame
