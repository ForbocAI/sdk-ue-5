#pragma once

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
  func::Maybe<FString> Status;
  func::Maybe<float> Progress;
  func::Maybe<bool> HasResults;
  func::Maybe<int32> ResultCount;
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
  FString Status;
  FString Progress;
  FString HasResults;
  FString ResultCount;
  FString HistoryCount;
  FString HistoryFirstSessionId;
  FString Loading;
  FString Error;
};

struct FGhostTestFixtures {
  FGhostTestLabels Labels;
  TArray<FGhostTestScenario> Scenarios;
};

} // namespace Testing::Ghost
