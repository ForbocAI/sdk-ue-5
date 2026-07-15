#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"
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

  FNPCStateLogEntry() : Timestamp(0) {}
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
