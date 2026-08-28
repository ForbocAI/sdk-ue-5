#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Components/Ghost/Identity/IdentityTypes.h"
#include "LifecycleTypes.generated.h"

USTRUCT(BlueprintType)
struct FGhostRunRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString TestSuite;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  int32 Duration;

  /** User Story: As a Ghost run caller, I need deterministic duration initialization before authored policy is applied. @fn FGhostRunRequest() */
  FGhostRunRequest()
      : Duration(FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646) {}
};

USTRUCT(BlueprintType)
struct FGhostRunResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString SessionId;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString RunStatus;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FString GhostName;

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  FGhostRuntimeIdentity RuntimeIdentity;
};

USTRUCT(BlueprintType)
struct FGhostStatus {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString SessionId;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString GhostName;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FGhostRuntimeIdentity RuntimeIdentity;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString TestSuite;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString Status;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Progress;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString StartedAt;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Duration;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") int32 Errors;

  /** User Story: As Ghost status state, I need numeric fields initialized before strict decoding assigns an atomic response. @fn FGhostStatus() */
  FGhostStatus() : Progress(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Duration(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), Errors(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

USTRUCT(BlueprintType)
struct FGhostStopResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") bool bStopped;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString StopStatus;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString StopSessionId;

  /** User Story: As Ghost lifecycle state, I need stop state to begin false until the API response proves completion. @fn FGhostStopResponse() */
  FGhostStopResponse() : bStopped(false) {}
};

USTRUCT(BlueprintType)
struct FGhostHistoryEntry {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString SessionId;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString GhostName;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FGhostRuntimeIdentity RuntimeIdentity;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString TestSuite;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString StartedAt;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString CompletedAt;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") FString Status;
  UPROPERTY(BlueprintReadOnly, Category = "Ghost") float PassRate;

  /** User Story: As Ghost history state, I need pass rate initialized before strict decoding assigns a complete record. @fn FGhostHistoryEntry() */
  FGhostHistoryEntry() : PassRate(FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF) {}
};

USTRUCT(BlueprintType)
struct FGhostHistoryResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Ghost")
  TArray<FGhostHistoryEntry> Sessions;
};
