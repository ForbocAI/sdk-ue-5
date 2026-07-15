#pragma once

#include "Features/Ghost/GhostTypes.h"

namespace TypeFactory {

inline FGhostConfig
GhostConfig(const FAgent &Agent,
            const TArray<FString> &Scenarios = TArray<FString>(),
            int32 MaxIterations = 100, bool bVerbose = false,
            const FString &ApiUrl = TEXT(""), const FString &ApiKey = TEXT(""),
            const FString &TestSuite = TEXT(""), int32 Duration = 300) {
  FGhostConfig Config;
  Config.Agent = Agent;
  Config.Scenarios = Scenarios;
  Config.MaxIterations = MaxIterations;
  Config.bVerbose = bVerbose;
  Config.ApiUrl = ApiUrl;
  Config.ApiKey = ApiKey;
  Config.TestSuite = TestSuite;
  Config.Duration = Duration;
  return Config;
}

inline FGhostRunRequest GhostRunRequest(const FString &TestSuite,
                                        int32 Duration = 300) {
  FGhostRunRequest Request;
  Request.TestSuite = TestSuite;
  Request.Duration = Duration;
  return Request;
}

} // namespace TypeFactory
