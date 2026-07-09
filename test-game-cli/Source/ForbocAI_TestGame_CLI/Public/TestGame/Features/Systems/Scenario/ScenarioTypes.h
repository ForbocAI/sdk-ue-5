#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Systems/Harness/Commands/CommandsTypes.h"

namespace TestGame {

enum class EEventType : uint8 { Stealth, Social, Escape, Persistence };

struct FScenarioStep {
  FString Id;
  FString Title;
  FString Description;
  EEventType EventType;
  TArray<FCommandSpec> Commands;
};

} // namespace TestGame
