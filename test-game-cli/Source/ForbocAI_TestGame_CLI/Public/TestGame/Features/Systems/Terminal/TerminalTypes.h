#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

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

  /** User Story: As a features systems terminal consumer, I need to invoke fuistate through a stable signature so the features systems terminal workflow remains explicit and composable. @fn FUIState() */
  FUIState() : Mode(EPlayMode::Autoplay) {
    Messages.Add(TEXT("SYSTEM_OVERRIDE :: terminal HUD online"));
  }
};

typedef FUIState FUiState;

struct FTranscriptState {
  TArray<FTranscriptEntry> Entries;
};

} // namespace TestGame
