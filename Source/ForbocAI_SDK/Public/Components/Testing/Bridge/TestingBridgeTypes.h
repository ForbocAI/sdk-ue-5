#pragma once

#include "Core/fp.hpp"
#include "Components/Contracts/ContractsTypes.h"

namespace Testing::Bridge {

enum class EBridgeTestActionKind : uint8 {
  ValidationRequested,
  ValidationSucceeded,
  ValidationFailed,
  ActivePresetsReceived,
  ActivePresetAdded,
  RulesetsReceived,
  PresetIdsReceived,
  ValidationCleared,
  Inspect,
  Count
};

struct FBridgeTestAction {
  EBridgeTestActionKind Kind;
  TArray<FDirectiveRuleSet> Rulesets;
  TArray<FString> PresetIds;
  func::Maybe<FString> TargetId;
  func::Maybe<bool> Valid;
  func::Maybe<FString> Reason;
};

struct FBridgeTestExpected {
  func::Maybe<FString> Status;
  func::Maybe<FString> Error;
  func::Maybe<bool> HasValidation;
  func::Maybe<bool> ValidationValid;
  func::Maybe<FString> ValidationReason;
  func::Maybe<int32> ActiveCount;
  func::Maybe<FString> ActiveFirstId;
  func::Maybe<int32> AvailableCount;
  func::Maybe<FString> AvailableFirstId;
  func::Maybe<FString> AvailableSelectedRulesetId;
  func::Maybe<int32> PresetIdCount;
};

struct FBridgeTestStep {
  FBridgeTestAction Action;
  FBridgeTestExpected Expected;
};

struct FBridgeTestScenario {
  FString Name;
  TArray<FBridgeTestStep> Steps;
};

struct FBridgeTestLabels {
  FString Suite;
  FString CaseName;
  FString RequiredField;
  FString ScenarioPresent;
  FString Status;
  FString Error;
  FString HasValidation;
  FString ValidationValid;
  FString ValidationReason;
  FString ActiveCount;
  FString ActiveFirstId;
  FString AvailableCount;
  FString AvailableFirstId;
  FString AvailableSelectedRulesetId;
  FString PresetIdCount;
};

struct FBridgeTestFixtures {
  FBridgeTestLabels Labels;
  TArray<FBridgeTestScenario> Scenarios;
};

} // namespace Testing::Bridge
