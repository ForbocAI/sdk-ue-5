// User Story: As a developer, I need this module to function.
#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Components/NPC/Agent/AgentTypes.h"
#include "GhostTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicGhostGhostTypesHOptionalDomainId;
} } }


USTRUCT(BlueprintType)
struct FGhostTestResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString Scenario;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  bool bPassed;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString ActualResponse;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString ErrorMessage;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString Screenshot;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Iteration;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int64 Duration;

  /** User Story: As a features ghost consumer, I need to invoke fghost test result through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostTestResult() */
  FGhostTestResult() : bPassed(false), Iteration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Duration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};
USTRUCT(BlueprintType)
struct FGhostHistoryEntry {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestSuite;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString StartedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString CompletedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString Status;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  float PassRate;

  /** User Story: As a features ghost consumer, I need to invoke fghost history entry through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostHistoryEntry() */
  FGhostHistoryEntry() : PassRate(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF) {}
};

USTRUCT(BlueprintType)
struct FGhostRunResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString RunStatus;
};

USTRUCT(BlueprintType)
struct FGhostRunRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestSuite;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Duration;

  /** User Story: As a features ghost consumer, I need to invoke fghost run request through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostRunRequest() */
  FGhostRunRequest() : Duration(FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {}
};

USTRUCT(BlueprintType)
struct FGhostConfig {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FAgent Agent;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TArray<FString> Scenarios;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 MaxIterations;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  bool bVerbose;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString ApiUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString ApiKey;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestSuite;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Duration;

  /** User Story: As a features ghost consumer, I need to invoke fghost config through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostConfig() */
  FGhostConfig() : MaxIterations(FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831), bVerbose(false), Duration(FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {}
};

USTRUCT(BlueprintType)
struct FGhostTestReport {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FGhostConfig Config;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TArray<FGhostTestResult> Results;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 TotalTests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 PassedTests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 FailedTests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 SkippedTests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int64 Duration;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  float Coverage;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TMap<FString, float> Metrics;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  float SuccessRate;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString Summary;

  /** User Story: As a features ghost consumer, I need to invoke fghost test report through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostTestReport() */
  FGhostTestReport()
      : TotalTests(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), PassedTests(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), FailedTests(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), SkippedTests(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA),
        Duration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Coverage(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF), SuccessRate(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF) {}
};

struct FGhost {
  FGhostConfig Config;
  bool bInitialized;

  /** User Story: As a features ghost consumer, I need to invoke fghost through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhost() */
  FGhost() : bInitialized(false) {}
};

USTRUCT(BlueprintType)
struct FGhostStatus {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString Status;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  float Progress;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString StartedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Duration;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Errors;

  /** User Story: As a features ghost consumer, I need to invoke fghost status through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostStatus() */
  FGhostStatus() : Progress(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF), Duration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Errors(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

USTRUCT(BlueprintType)
struct FGhostResultRecord {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestName;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  bool bTestPassed;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int64 TestDuration;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestError;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestScreenshot;

  /** User Story: As a features ghost consumer, I need to invoke fghost result record through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostResultRecord() */
  FGhostResultRecord() : bTestPassed(false), TestDuration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

USTRUCT(BlueprintType)
struct FGhostResults {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 TotalTests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Passed;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Failed;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Skipped;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int64 Duration;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TArray<FGhostResultRecord> Tests;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  float Coverage;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TMap<FString, float> Metrics;

  /** User Story: As a features ghost consumer, I need to invoke fghost results through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostResults() */
  FGhostResults()
      : TotalTests(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Passed(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Failed(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Skipped(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Duration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA),
        Coverage(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF) {}
};

USTRUCT(BlueprintType)
struct FGhostStopResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  bool bStopped;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString StopStatus;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString StopSessionId;

  /** User Story: As a features ghost consumer, I need to invoke fghost stop response through a stable signature so the features ghost workflow remains explicit and composable. @fn FGhostStopResponse() */
  FGhostStopResponse() : bStopped(false) {}
};

USTRUCT(BlueprintType)
struct FGhostHistoryResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TArray<FGhostHistoryEntry> Sessions;
};
