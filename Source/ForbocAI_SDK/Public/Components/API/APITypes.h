#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "APITypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicFeaturesAPIAPITypesHOptionalDomainId;
} } }

/**
 * API status response.
 * User Story: As an SDK integrator, I need the API health response contract
 * colocated with the API feature rather than the Core primitive layer.
 */
USTRUCT(BlueprintType)
struct FApiStatusResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString Status;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString Version;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  int32 InferenceLatencyBudgetMs{};

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString Message;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString SlmArtifactSha256;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString SlmStatus;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString SlmVersion;

  UPROPERTY(BlueprintReadOnly, Category = "System")
  FString SlotContractVersion;

  /**
   * User Story: As a features api consumer, I need to invoke fapi status response through a stable signature so the features api workflow remains explicit and composable.
   * @fn FApiStatusResponse() = default
   */
  FApiStatusResponse() = default;
};
