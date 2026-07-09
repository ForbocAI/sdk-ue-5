#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/HarnessTypes.h"

namespace TestGame {
namespace CommandSurface {

struct FCommandOutput {
  ETranscriptStatus Status;
  FString Output;
  FString RoutedThrough;
};

struct FAliasState {
  TMap<FString, FString> NpcAliases;
  TMap<FString, FString> GhostAliases;
  FString LastGhostSessionId;
  FString NpcCreateAliasRule;
  FString BridgeValidateCommandRule;
};

} // namespace CommandSurface
} // namespace TestGame
