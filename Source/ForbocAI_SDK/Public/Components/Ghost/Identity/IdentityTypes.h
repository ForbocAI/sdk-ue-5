#pragma once

#include "CoreMinimal.h"
#include "IdentityTypes.generated.h"

USTRUCT(BlueprintType)
struct FGhostRuntimeIdentity {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString ApiVersion;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SlmStatus;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SlmVersion;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SlotContractVersion;
};
