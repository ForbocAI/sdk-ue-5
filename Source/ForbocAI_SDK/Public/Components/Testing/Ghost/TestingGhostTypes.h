#pragma once

#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Components/Ghost/Results/ResultsTypes.h"
#include "Core/fp.hpp"
#include "CoreMinimal.h"

namespace Testing::Ghost {

enum class EGhostTestActionKind : uint8 {
  Start,
  Progress,
  Complete,
  Fail,
  LoadHistory,
  Clear,
  Inspect,
  Count
};

struct FGhostTestAction {
  EGhostTestActionKind Kind;
  func::Maybe<FString> SessionId;
  func::Maybe<FString> Status;
  func::Maybe<float> Progress;
  func::Maybe<FString> Error;
  TArray<FString> ResultScenarios;
  TArray<FString> HistorySessionIds;
};

struct FGhostTestExpected {
  func::Maybe<FString> ActiveSessionId;
  func::Maybe<bool> IdentityPreserved;
  func::Maybe<FString> Status;
  func::Maybe<float> Progress;
  func::Maybe<bool> HasResults;
  func::Maybe<int32> ResultCount;
  func::Maybe<bool> ResultsPreserved;
  func::Maybe<int32> HistoryCount;
  func::Maybe<FString> HistoryFirstSessionId;
  func::Maybe<bool> Loading;
  func::Maybe<FString> Error;
};

struct FGhostTestStep {
  FGhostTestAction Action;
  FGhostTestExpected Expected;
};

struct FGhostTestScenario {
  FString Name;
  TArray<FGhostTestStep> Steps;
};

struct FGhostTestLabels {
  FString Suite;
  FString CaseName;
  FString RequiredField;
  FString ScenarioPresent;
  FString ActiveSessionId;
  FString IdentityPreserved;
  FString Status;
  FString Progress;
  FString HasResults;
  FString ResultCount;
  FString ResultsPreserved;
  FString HistoryCount;
  FString HistoryFirstSessionId;
  FString Loading;
  FString Error;
};

struct FGhostTestFixtures {
  FGhostTestLabels Labels;
  FGhostRunResponse Identity;
  FGhostResults ResultsTemplate;
  FGhostHistoryEntry HistoryTemplate;
  TArray<FGhostTestScenario> Scenarios;
};

} // namespace Testing::Ghost
