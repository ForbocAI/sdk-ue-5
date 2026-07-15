#pragma once

#include "Core/fp.hpp"
#include "Features/Contracts/ContractsTypes.h"

namespace Testing::Memory {

enum class EMemoryTestActionKind : uint8 {
  StoreStarted,
  StoreSucceeded,
  StoreFailed,
  RecallStarted,
  RecallSucceeded,
  RecallFailed,
  Cleared,
  Inspect,
  Count
};

struct FMemoryTestAction {
  EMemoryTestActionKind Kind;
  TArray<FMemoryItem> Items;
  func::Maybe<FString> Error;
  func::Maybe<FString> TargetId;
};

struct FMemoryTestExpected {
  func::Maybe<FString> StorageStatus;
  func::Maybe<FString> RecallStatus;
  func::Maybe<FString> Error;
  func::Maybe<int32> MemoryCount;
  func::Maybe<int32> RecalledCount;
  func::Maybe<FString> FirstRecalledId;
  func::Maybe<FString> SelectedText;
};

struct FMemoryTestStep {
  FMemoryTestAction Action;
  FMemoryTestExpected Expected;
};

struct FMemoryTestScenario {
  FString Name;
  TArray<FMemoryTestStep> Steps;
};

struct FMemoryTestLabels {
  FString Suite;
  FString CaseName;
  FString RequiredField;
  FString ScenarioPresent;
  FString StorageStatus;
  FString RecallStatus;
  FString Error;
  FString MemoryCount;
  FString RecalledCount;
  FString FirstRecalledId;
  FString SelectedText;
};

struct FMemoryTestFixtures {
  FMemoryTestLabels Labels;
  TArray<FMemoryTestScenario> Scenarios;
};

} // namespace Testing::Memory
