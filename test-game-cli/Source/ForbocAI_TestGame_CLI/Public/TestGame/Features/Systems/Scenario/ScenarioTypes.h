#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Commands/CommandsTypes.h"

namespace TestGame {

enum class EEventType : uint8 {
  Unknown,
  Stealth,
  Social,
  Escape,
  Persistence
};

struct FScenarioStep {
  FString Id;
  FString Title;
  FString Description;
  EEventType EventType;
  TArray<FCommandSpec> Commands;

  FScenarioStep() : EventType(EEventType::Unknown) {}
};

struct FScenarioSliceState {
  TArray<FScenarioStep> Steps;

  bool operator==(const FScenarioSliceState &Other) const {
    return Steps.Num() == Other.Steps.Num();
  }
};

typedef FScenarioSliceState FScenarioState;

} // namespace TestGame
