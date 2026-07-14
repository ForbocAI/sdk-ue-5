#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Features/Protocol/ProtocolTypes.h"
#include "RequestsTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicFeaturesProtocolRequestsRequestsTypesHOptionalDomainId;
} } }


USTRUCT(BlueprintType)
struct FNPCActorInfo {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString NpcId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Persona;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState Data;
};
USTRUCT(BlueprintType)
struct FDecisionIntent {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Goal;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ActionType;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Target;

  FDecisionIntent() : Goal(TEXT("")), ActionType(TEXT("SPEAK")), Target(TEXT("")) {}
};
USTRUCT(BlueprintType)
struct FReasoningOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ReasoningText;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ResponseText;

  FReasoningOutput() : ReasoningText(TEXT("")), ResponseText(TEXT("")) {}
};

USTRUCT(BlueprintType)
struct FNPCProcessTape {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Observation;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ContextJson;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState NpcState;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Persona;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasActor;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FNPCActorInfo Actor;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FRecalledMemory> Memories;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString RulesetId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bVectorQueried;

  /** Decision step result — intent selected by the local decision handler. */
  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FDecisionIntent DecisionIntent;

  /** Whether the decision step has been completed in this tape. */
  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bDecisionCompleted;

  /** Reasoning step result — output from the SLM. */
  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FReasoningOutput ReasoningOutput;

  /** Whether the reasoning step has been completed in this tape. */
  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bReasoningCompleted;

  FNPCProcessTape()
      : bHasActor(false), bVectorQueried(false), bDecisionCompleted(false),
        bReasoningCompleted(false) {}
};

USTRUCT(BlueprintType)
struct FNPCProcessRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FNPCProcessTape Tape;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString PreviousResult;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasPreviousResult;

  FNPCProcessRequest() : bHasPreviousResult(false) {}
};

USTRUCT(BlueprintType)
struct FNPCProcessResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FNPCInstruction Instruction;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FNPCProcessTape Tape;
};

USTRUCT(BlueprintType)
struct FDirectiveRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Observation;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState NpcState;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ContextJson;
};

USTRUCT(BlueprintType)
struct FDirectiveResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FMemoryRecallInstruction recallMemory;
};

USTRUCT(BlueprintType)
struct FVerdictResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bValid;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Signature;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FMemoryStoreInstruction> storeMemory;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState StateDelta;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentAction Action;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasAction;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Dialogue;

  FVerdictResponse() : bValid(true), bHasAction(false) {}
};
