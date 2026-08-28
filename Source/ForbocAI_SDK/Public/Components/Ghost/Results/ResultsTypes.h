#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Components/Ghost/Identity/IdentityTypes.h"
#include "ResultsTypes.generated.h"

USTRUCT(BlueprintType)
struct FGhostEvidenceSummary {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 CompletedProcessTurns{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 ValidCognitionOutcomes{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 InvalidCognitionOutcomes{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 SignedCognitionOutcomes{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 ThoughtResults{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 ReasoningResults{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 ConsistentReasoningResults{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 DiagnosticResults{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 GroundedDiagnosticResults{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Contradictions{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 UnsupportedClaims{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Failures{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Timeouts{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 UniqueObservations{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 ChoicePoints{};
};

USTRUCT(BlueprintType)
struct FGhostResultRecord {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString TestName;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") bool bTestPassed;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int64 TestDuration;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString TestError;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString TestScreenshot;

  /** User Story: As a Ghost evidence record, I need deterministic scalar initialization before strict decoding. @fn FGhostResultRecord() */
  FGhostResultRecord() : bTestPassed(false), TestDuration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

USTRUCT(BlueprintType)
struct FGhostResults {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString SessionId;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString GhostName;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FGhostRuntimeIdentity RuntimeIdentity;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FGhostEvidenceSummary Evidence;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") TArray<FString> EvidenceDimensions;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") TArray<FString> EvaluationDimensions;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 TotalTests{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Passed{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Failed{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Skipped{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int64 Duration{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") TArray<FGhostResultRecord> Tests;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") float Coverage{};
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") TMap<FString, float> Metrics;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString Verdict;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString Summary;
};
