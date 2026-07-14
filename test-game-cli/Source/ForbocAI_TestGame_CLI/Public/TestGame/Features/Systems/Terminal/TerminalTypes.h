#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Commands/CommandsTypes.h"

namespace TestGame {

enum class ETranscriptStatus : uint8 { Ok, Error };

struct FTranscriptEntry {
  FString Id;
  FString ScenarioId;
  ECommandGroup CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  ETranscriptStatus Status;
  FString Output;
  FString Timestamp;
};

enum class EPlayMode : uint8 { Manual, Autoplay };

struct FUIState {
  EPlayMode Mode;
  TArray<FString> Messages;

  FUIState() : Mode(EPlayMode::Autoplay) {
    Messages.Add(TEXT("SYSTEM_OVERRIDE :: terminal HUD online"));
  }
};

typedef FUIState FUiState;

struct FTranscriptState {
  TArray<FTranscriptEntry> Entries;
};

} // namespace TestGame
