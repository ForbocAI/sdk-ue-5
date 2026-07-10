#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

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

  FApiStatusResponse() {}
};

namespace TypeFactory {

inline FApiStatusResponse ApiStatusResponse(FString Status, FString Version) {
  FApiStatusResponse R;
  R.Status = MoveTemp(Status);
  R.Version = MoveTemp(Version);
  return R;
}

} // namespace TypeFactory
