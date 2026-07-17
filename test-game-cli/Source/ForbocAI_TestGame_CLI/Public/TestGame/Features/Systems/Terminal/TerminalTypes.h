#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace TestGame {

struct FTranscriptEntry {
  FString Id;
  FString ScenarioId;
  FString CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  FString Status;
  FString Output;
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

struct FTerminalData {
  FUIState initialState;
  FTerminalMessages messages;
  TArray<FString> usage;
  FTerminalGridData grid;
  TArray<FTerminalNpcGlyph> npcGlyphs;
  FString unknownNpcGlyph;
  FTerminalLevels levels;
  FTerminalTranscriptData transcript;
  FTerminalTokens tokens;
};

typedef FUIState FUiState;

struct FTranscriptState {
  TArray<FTranscriptEntry> Entries;
};

} // namespace TestGame
