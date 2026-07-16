#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Features/Memory/MemoryTypes.h"
#include "NPC/NPCBaseTypes.h"
#include "ProtocolTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicProtocolProtocolTypesHOptionalDomainId;
} } }


UENUM(BlueprintType)
enum class EDirectiveStatus : uint8 { Running, Completed, Failed };

USTRUCT(BlueprintType)
struct FDirectiveRun {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString NpcId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Observation;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  EDirectiveStatus Status{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int64 StartedAt{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int64 CompletedAt{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString MemoryRecallQuery;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int32 MemoryRecallLimit{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float MemoryRecallThreshold{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bVerdictValid{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString VerdictDialogue;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString VerdictActionType;

  /**
   * User Story: As a features protocol consumer, I need to invoke fdirective run through a stable signature so the features protocol workflow remains explicit and composable.
   * @fn FDirectiveRun() = default
   */
  FDirectiveRun() = default;
};

USTRUCT(BlueprintType)
struct FMemoryRecallInstruction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Query;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int32 Limit{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Threshold{};

  /**
   * User Story: As a features protocol consumer, I need to invoke fmemory recall instruction through a stable signature so the features protocol workflow remains explicit and composable.
   * @fn FMemoryRecallInstruction() = default
   */
  FMemoryRecallInstruction() = default;
};

USTRUCT(BlueprintType)
struct FMemoryStoreInstruction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Text;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Type;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Importance{};

  /**
   * User Story: As a features protocol consumer, I need to invoke fmemory store instruction through a stable signature so the features protocol workflow remains explicit and composable.
   * @fn FMemoryStoreInstruction() = default
   */
  FMemoryStoreInstruction() = default;
};

USTRUCT(BlueprintType)
struct FRecalledMemory {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Text;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Type;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Importance{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Similarity{};

  /**
   * User Story: As a features protocol consumer, I need to invoke frecalled memory through a stable signature so the features protocol workflow remains explicit and composable.
   * @fn FRecalledMemory() = default
   */
  FRecalledMemory() = default;
};

USTRUCT(BlueprintType)
struct FPromptConstraints {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasMaxTokens{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int32 MaxTokens{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasTemperature{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Temperature{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasStop{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FString> Stop;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasRepeatPenalty{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float RepeatPenalty{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasSeed{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int32 Seed{};

  /**
   * User Story: As a protocol consumer, I need absent inference constraints represented explicitly so zero values are never confused with omission.
   * @fn FPromptConstraints() = default
   */
  FPromptConstraints() = default;
};

USTRUCT(BlueprintType)
struct FRuleVerdictEntry {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString RuleId;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Verdict;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasReason{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Reason;

  /**
   * User Story: As a rule-audit consumer, I need optional block reasons represented without manufacturing empty reasons.
   * @fn FRuleVerdictEntry() = default
   */
  FRuleVerdictEntry() = default;
};

USTRUCT(BlueprintType)
struct FRuleAudit {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Preset;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FRuleVerdictEntry> AppliedRules;
};

UENUM(BlueprintType)
enum class ENPCInstructionType : uint8 {
  IdentifyActor,
  QueryVector,
  Decision,
  Reasoning,
  Finalize
};

USTRUCT(BlueprintType)
struct FNPCInstruction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  ENPCInstructionType Type{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Query;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  int32 Limit{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  float Threshold{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bValid{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Signature;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  TArray<FMemoryStoreInstruction> storeMemory;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentState StateTransform;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FAgentAction Action;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasAction{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FString Dialogue;

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  bool bHasRuleAudit{};

  UPROPERTY(BlueprintReadOnly, Category = "Protocol")
  FRuleAudit RuleAudit;

  /**
   * User Story: As a features protocol consumer, I need to invoke fnpcinstruction through a stable signature so the features protocol workflow remains explicit and composable.
   * @fn FNPCInstruction() = default
   */
  FNPCInstruction() = default;
};

namespace TypeFactory {

/** User Story: As a features protocol consumer, I need to invoke memory recall instruction through a stable signature so the features protocol workflow remains explicit and composable. @fn inline FMemoryRecallInstruction MemoryRecallInstruction(const FString &Query, int32 Limit, float Threshold) */
inline FMemoryRecallInstruction
MemoryRecallInstruction(const FString &Query, int32 Limit,
                        float Threshold) {
  FMemoryRecallInstruction Instruction;
  Instruction.Query = Query;
  Instruction.Limit = Limit;
  Instruction.Threshold = Threshold;
  return Instruction;
}

/** User Story: As a features protocol consumer, I need to invoke memory store instruction through a stable signature so the features protocol workflow remains explicit and composable. @fn inline FMemoryStoreInstruction MemoryStoreInstruction(const FString &Text, const FString &Type, float Importance) */
inline FMemoryStoreInstruction
MemoryStoreInstruction(const FString &Text,
                       const FString &Type, float Importance) {
  FMemoryStoreInstruction Instruction;
  Instruction.Text = Text;
  Instruction.Type = Type;
  Instruction.Importance = Importance;
  return Instruction;
}

} // namespace TypeFactory
