#pragma once

#include "Core/fp.hpp"

namespace Testing::CLI::Invocation {

struct FInvocationTestLabels {
  FString NodeCommandCount;
  FString CommandMatched;
  FString CommandKey;
  FString Arguments;
  FString ApiUrl;
  FString ApiKey;
};

struct FInvocationTestScenario {
  FString Name;
  FString CommandletParams;
  bool bExpectedMatched;
  func::Maybe<FString> ExpectedKey;
  TArray<FString> ExpectedArgs;
  func::Maybe<FString> ExpectedApiUrl;
  func::Maybe<FString> ExpectedApiKey;
};

struct FInvocationTestFixtures {
  FInvocationTestLabels Labels;
  TArray<FInvocationTestScenario> Scenarios;
};

} // namespace Testing::CLI::Invocation
