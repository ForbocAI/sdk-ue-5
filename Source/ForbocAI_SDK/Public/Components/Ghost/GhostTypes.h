// User Story: As a Ghost caller, I need only client configuration retained in the SDK domain.
#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Components/NPC/Agent/AgentTypes.h"
#include "GhostTypes.generated.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicGhostGhostTypesHOptionalDomainId;
} } }

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

  /** User Story: As a Ghost caller, I need deterministic value initialization before authored configuration is applied. @fn FGhostConfig() */
  FGhostConfig()
      : MaxIterations(FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831),
        bVerbose(false),
        Duration(FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {}
};

struct FGhost {
  FGhostConfig Config;
  bool bInitialized;

  /** User Story: As a Ghost owner, I need explicit initialization state around the client configuration. @fn FGhost() */
  FGhost() : bInitialized(false) {}
};
