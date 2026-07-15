#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"

namespace Testing::Directive {

enum class EDirectiveTestActionKind : uint8 {
  Start,
  Receive,
  Validate,
  Fail,
  ClearNpc,
  Count
};

struct FDirectiveTestAction {
  EDirectiveTestActionKind Kind;
  func::Maybe<FString> Id;
  func::Maybe<FString> NpcId;
  func::Maybe<FString> Observation;
  func::Maybe<FString> Query;
  func::Maybe<int32> Limit;
  func::Maybe<float> Threshold;
  func::Maybe<bool> VerdictValid;
  func::Maybe<FString> Dialogue;
  func::Maybe<bool> HasAction;
  func::Maybe<FString> ActionType;
  func::Maybe<FString> ActionTarget;
  func::Maybe<FString> ActionReason;
  func::Maybe<FString> Error;
};

struct FDirectiveTestRunExpected {
  FString Id;
  bool bExists;
  func::Maybe<int32> Status;
  func::Maybe<FString> NpcId;
  func::Maybe<FString> Observation;
  func::Maybe<FString> Error;
  func::Maybe<FString> RecallQuery;
  func::Maybe<int32> RecallLimit;
  func::Maybe<float> RecallThreshold;
  func::Maybe<bool> VerdictValid;
  func::Maybe<FString> Dialogue;
  func::Maybe<FString> ActionType;
};

struct FDirectiveTestExpected {
  int32 DirectiveCount;
  FString ActiveId;
  TArray<FDirectiveTestRunExpected> Runs;
};

struct FDirectiveTestScenario {
  FString Name;
  TArray<FDirectiveTestAction> Actions;
  FDirectiveTestExpected Expected;
};

struct FDirectiveTestLabels {
  FString RequiredField;
  FString ScenarioPresent;
  FString DirectiveCount;
  FString ActiveId;
  FString ActivePresence;
  FString RunPresence;
  FString Status;
  FString NpcId;
  FString Observation;
  FString Error;
  FString RecallQuery;
  FString RecallLimit;
  FString RecallThreshold;
  FString VerdictValid;
  FString Dialogue;
  FString ActionType;
};

struct FDirectiveTestFixtures {
  FDirectiveTestLabels Labels;
  TArray<FDirectiveTestScenario> Scenarios;
};

} // namespace Testing::Directive
