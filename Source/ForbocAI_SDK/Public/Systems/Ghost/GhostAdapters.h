#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Components/Ghost/GhostTypes.h"

namespace TypeFactory {

/**
 * User Story: As a features ghost consumer, I need to invoke ghost config through a stable signature so the features ghost workflow remains explicit and composable.
 * @fn inline FGhostConfig GhostConfig(const FAgent &Agent, const TArray<FString> &Scenarios = TArray<FString>(), int32 MaxIterations = FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831, bool bVerbose = false, const FString &ApiUrl = TEXT(""), const FString &ApiKey = TEXT(""), const FString &TestSuite = TEXT(""), int32 Duration = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646)
 */
inline FGhostConfig
GhostConfig(const FAgent &Agent,
            const TArray<FString> &Scenarios = TArray<FString>(),
            int32 MaxIterations = FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831, bool bVerbose = false,
            const FString &ApiUrl = TEXT(""), const FString &ApiKey = TEXT(""),
            const FString &TestSuite = TEXT(""), int32 Duration = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {
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

/**
 * User Story: As a features ghost consumer, I need to invoke ghost run request through a stable signature so the features ghost workflow remains explicit and composable.
 * @fn inline FGhostRunRequest GhostRunRequest(const FString &TestSuite, int32 Duration = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646)
 */
inline FGhostRunRequest GhostRunRequest(const FString &TestSuite,
                                        int32 Duration = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {
  FGhostRunRequest Request;
  Request.TestSuite = TestSuite;
  Request.Duration = Duration;
  return Request;
}

} // namespace TypeFactory
