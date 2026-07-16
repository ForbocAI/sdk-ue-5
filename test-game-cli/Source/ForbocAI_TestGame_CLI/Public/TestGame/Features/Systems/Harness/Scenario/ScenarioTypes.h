#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

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

  /** User Story: As a systems harness scenario consumer, I need to invoke fscenario step through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn FScenarioStep() */
  FScenarioStep() : EventType(EEventType::Unknown) {}
};

struct FScenarioSliceState {
  TArray<FScenarioStep> Steps;

  /** User Story: As a systems harness scenario consumer, I need to compare values for equality through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn bool operator==(const FScenarioSliceState &Other) const */
  bool operator==(const FScenarioSliceState &Other) const {
    return Steps.Num() == Other.Steps.Num();
  }
};

typedef FScenarioSliceState FScenarioState;

} // namespace TestGame
