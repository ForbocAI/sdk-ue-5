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
  bool bHasStructuredPersona;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState Data;

  /** User Story: As a process-tape consumer, I need actor persona absence represented explicitly instead of inferred from an empty string. @fn FNPCActorInfo() */
  FNPCActorInfo() : bHasStructuredPersona(false) {}
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

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasMetadata{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString MetadataJson;

  /**
   * User Story: As a features protocol requests consumer, I need to invoke fdecision intent through a stable signature so the features protocol requests workflow remains explicit and composable.
   * @fn FDecisionIntent() = default
   */
  FDecisionIntent() = default;
};
USTRUCT(BlueprintType)
struct FReasoningOutput {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ReasoningText;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString ResponseText;

  /**
   * User Story: As a features protocol requests consumer, I need to invoke freasoning output through a stable signature so the features protocol requests workflow remains explicit and composable.
   * @fn FReasoningOutput() = default
   */
  FReasoningOutput() = default;
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
  bool bHasStructuredPersona;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasSamplerProfile;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString SamplerProfile;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasActor;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FNPCActorInfo Actor;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FRecalledMemory> Memories;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString RulesetId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasRulesetId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bVectorQueried;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasVectorQueried;

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

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasPrompt;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Prompt;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasConstraints;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FPromptConstraints Constraints;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasGeneratedOutput;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString GeneratedOutput;

  /** User Story: As a features protocol requests consumer, I need to invoke fnpcprocess tape through a stable signature so the features protocol requests workflow remains explicit and composable. @fn FNPCProcessTape() */
  FNPCProcessTape()
      : bHasStructuredPersona(false), bHasSamplerProfile(false),
        bHasActor(false), bHasRulesetId(false), bVectorQueried(false),
        bHasVectorQueried(false), bDecisionCompleted(false),
        bReasoningCompleted(false), bHasPrompt(false),
        bHasConstraints(false), bHasGeneratedOutput(false) {}
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

  /** User Story: As a features protocol requests consumer, I need to invoke fnpcprocess request through a stable signature so the features protocol requests workflow remains explicit and composable. @fn FNPCProcessRequest() */
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

  /** User Story: As a features protocol requests consumer, I need to invoke fverdict response through a stable signature so the features protocol requests workflow remains explicit and composable. @fn FVerdictResponse() */
  FVerdictResponse() : bValid(true), bHasAction(false) {}
};
