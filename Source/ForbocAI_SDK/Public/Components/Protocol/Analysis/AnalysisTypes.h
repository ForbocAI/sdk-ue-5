#pragma once

#include "CoreMinimal.h"
#include "AnalysisTypes.generated.h"

USTRUCT(BlueprintType)
struct FDecisionIntent {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Goal;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ActionType;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Target;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasMetadata{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString MetadataJson;
};

USTRUCT(BlueprintType)
struct FReasoningOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ReasoningText;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ResponseText;
};
