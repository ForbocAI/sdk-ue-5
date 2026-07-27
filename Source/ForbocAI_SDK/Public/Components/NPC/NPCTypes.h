#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Core/rtk.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Components/Contracts/ContractsTypes.h"
#include "NPCTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicNPCNPCTypesHOptionalDomainId;
} } }


USTRUCT(BlueprintType)
struct FNPCHistoryEntry {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FString Role;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FString Content;
};

USTRUCT(BlueprintType)
struct FNPCStateLogEntry {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  int64 Timestamp;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FAgentState Delta;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FAgentState State;

  /** User Story: As a features npc consumer, I need to invoke fnpcstate log entry through a stable signature so the features npc workflow remains explicit and composable. @fn FNPCStateLogEntry() */
  FNPCStateLogEntry() : Timestamp(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

USTRUCT(BlueprintType)
struct FNPCInternalState {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FString Persona;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FAgentState State;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  TArray<FNPCHistoryEntry> History;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  bool bIsBlocked;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  FString BlockReason;

  UPROPERTY(BlueprintReadOnly, Category = "NPC")
  TArray<FNPCStateLogEntry> StateLog;

  /** User Story: As a features npc consumer, I need to invoke fnpcinternal state through a stable signature so the features npc workflow remains explicit and composable. @fn FNPCInternalState() */
  FNPCInternalState() : bIsBlocked(false) {}
};

struct FSetNPCInfoPayload {
  FNPCInternalState Info;
  int64 Timestamp;
};

struct FSetNPCStatePayload {
  FString Id;
  FAgentState State;
  int64 Timestamp;
};

struct FUpdateNPCStatePayload {
  FString Id;
  FAgentState Delta;
  int64 Timestamp;
};

struct FAddToHistoryPayload {
  FString Id;
  FString Role;
  FString Content;
};

struct FSetHistoryPayload {
  FString Id;
  TArray<FNPCHistoryEntry> History;
};

struct FNPCActionReceivedPayload {
  FString Id;
  FAgentAction Action;
};

struct FBlockActionPayload {
  FString Id;
  FString Reason;
};

namespace NPCSlice {

using ::FNPCHistoryEntry;
using ::FNPCInternalState;
using ::FNPCStateLogEntry;

struct FNPCSliceState {
  rtk::EntityState<FNPCInternalState> Entities;
  FString ActiveNpcId;
};

} // namespace NPCSlice
