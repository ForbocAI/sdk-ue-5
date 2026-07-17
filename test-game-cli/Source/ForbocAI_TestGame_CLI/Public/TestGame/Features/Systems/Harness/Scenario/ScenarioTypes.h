#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Command/CommandTypes.h"

namespace TestGame {

struct FScenarioStep {
  FString Id;
  FString Title;
  FString Description;
  FString EventType;
  TArray<FCommandSpec> Commands;
};

struct FScenarioContractPayload {
  TArray<FString> RequiredCommandGroups;
  TArray<FScenarioStep> Steps;
};

struct FScenarioSliceState {
  TArray<FString> RequiredCommandGroups;
  TArray<FScenarioStep> Steps;

  /** User Story: As a systems harness scenario consumer, I need to compare values for equality through a stable signature so the systems harness scenario workflow remains explicit and composable. @fn bool operator==(const FScenarioSliceState &Other) const */
  bool operator==(const FScenarioSliceState &Other) const {
    return RequiredCommandGroups == Other.RequiredCommandGroups &&
           Steps.Num() == Other.Steps.Num();
  }
};

typedef FScenarioSliceState FScenarioState;

} // namespace TestGame
