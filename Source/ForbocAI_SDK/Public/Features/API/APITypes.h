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

  /** User Story: As a features api consumer, I need to invoke fapi status response through a stable signature so the features api workflow remains explicit and composable. @fn FApiStatusResponse() */
  FApiStatusResponse() {}
};

namespace TypeFactory {

/** User Story: As a features api consumer, I need to invoke api status response through a stable signature so the features api workflow remains explicit and composable. @fn inline FApiStatusResponse ApiStatusResponse(FString Status, FString Version) */
inline FApiStatusResponse ApiStatusResponse(FString Status, FString Version) {
  FApiStatusResponse R;
  R.Status = MoveTemp(Status);
  R.Version = MoveTemp(Version);
  return R;
}

} // namespace TypeFactory
