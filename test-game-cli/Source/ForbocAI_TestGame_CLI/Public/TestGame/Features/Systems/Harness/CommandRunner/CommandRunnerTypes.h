#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace CommandRunner {

struct FCommandOutput {
  ETranscriptStatus Status;
  FString Output;
  FString RoutedThrough;

  FCommandOutput() : Status(ETranscriptStatus::Error) {}
  FCommandOutput(ETranscriptStatus InStatus, FString InOutput,
                 FString InRoutedThrough)
      : Status(InStatus), Output(MoveTemp(InOutput)),
        RoutedThrough(MoveTemp(InRoutedThrough)) {}
};

struct FAliasState {
  TMap<FString, FString> NpcAliases;
  FString NpcCreateAliasRule;
  FString BridgeValidateCommandRule;
};

} // namespace CommandRunner
} // namespace TestGame
