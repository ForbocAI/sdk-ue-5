#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "NPC/NPCBaseTypes.h"
#include "BridgeTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicBridgeBridgeTypesHOptionalDomainId;
} } }

enum class EBridgeStatus : uint8 {
  Idle,
  Validating,
  LoadingPreset,
  Error,
  Count
};


/**
 * Validation Result — Immutable data.
 * User Story: As bridge validation flows, I need one immutable result shape so
 * success and failure outcomes can move through reducers consistently.
 */
USTRUCT(BlueprintType)
struct FValidationResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  bool bValid;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Reason;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FAgentAction CorrectedAction;

  /** User Story: As a features bridge consumer, I need to invoke fvalidation result through a stable signature so the features bridge workflow remains explicit and composable. @fn FValidationResult() */
  FValidationResult() : bValid(false) {}
};

/**
 * Validation Context
 * User Story: As bridge validation inputs, I need a dedicated context payload
 * so NPC, world, and constraint state are passed together.
 */
USTRUCT(BlueprintType)
struct FBridgeValidationContext {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString NpcStateJson;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString WorldStateJson;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString ConstraintsJson;

  /** User Story: As a features bridge consumer, I need to invoke fbridge validation context through a stable signature so the features bridge workflow remains explicit and composable. @fn FBridgeValidationContext() */
  FBridgeValidationContext() {}
};

/**
 * Bridge Rule
 * User Story: As bridge rule catalogs, I need a typed rule model so server and
 * local validation rules can be represented in one shared shape.
 */
USTRUCT(BlueprintType)
struct FBridgeRule {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString RuleName;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString RuleDescription;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  TArray<FString> RuleActionTypes;

  /** User Story: As a features bridge consumer, I need to invoke fbridge rule through a stable signature so the features bridge workflow remains explicit and composable. @fn FBridgeRule() */
  FBridgeRule() {}
};

USTRUCT(BlueprintType)
struct FDirectiveRuleCondition {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Key;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Value;

  /** User Story: As a directive-rule consumer, I need a typed condition pair so API tuple values do not leak into feature code. @fn FDirectiveRuleCondition() */
  FDirectiveRuleCondition() {}
};

USTRUCT(BlueprintType)
struct FDirectiveRule {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString RuleId;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  TArray<FDirectiveRuleCondition> Conditions;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Action;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Reason;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Target;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  int32 Priority;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString ObservationPattern;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString PromptSuffix;

  /** User Story: As a directive-rule consumer, I need the full API rule contract represented in the feature domain so registration and catalog reads are lossless. @fn FDirectiveRule() */
  FDirectiveRule() : Priority(0) {}
};

/**
 * Directive Rule Set
 * User Story: As bridge preset management, I need a ruleset model so grouped
 * validation rules can be stored, selected, and exchanged consistently.
 */
USTRUCT(BlueprintType)
struct FDirectiveRuleSet {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString RulesetId;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  TArray<FDirectiveRule> RulesetRules;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FString Template;

  /** User Story: As a features bridge consumer, I need to invoke fdirective rule set through a stable signature so the features bridge workflow remains explicit and composable. @fn FDirectiveRuleSet() */
  FDirectiveRuleSet() {}
};

USTRUCT(BlueprintType)
struct FBridgeValidateRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FAgentAction Action;

  UPROPERTY(BlueprintReadOnly, Category = "Bridge")
  FBridgeValidationContext Context;
};

namespace TypeFactory {

/**
 * Builds a successful bridge validation result.
 * User Story: As bridge validation, I need a factory for success results so
 * reducers and callers can construct valid outcomes consistently.
 * @fn inline FValidationResult Valid(FString Reason)
 */
inline FValidationResult Valid(FString Reason) {
  FValidationResult R;
  R.bValid = true;
  R.Reason = MoveTemp(Reason);
  return R;
}

/**
 * Builds a failed bridge validation result.
 * User Story: As bridge validation, I need a factory for invalid results so
 * rejection reasons are carried through one shared shape.
 * @fn inline FValidationResult Invalid(FString Reason)
 */
inline FValidationResult Invalid(FString Reason) {
  FValidationResult R;
  R.bValid = false;
  R.Reason = MoveTemp(Reason);
  return R;
}

/**
 * Builds the bridge validation request payload.
 * User Story: As bridge API calls, I need a request factory so action and
 * context are packaged consistently before dispatch.
 * @fn inline FBridgeValidateRequest BridgeValidateRequest(const FAgentAction &Action, const FBridgeValidationContext &Context)
 */
inline FBridgeValidateRequest
BridgeValidateRequest(const FAgentAction &Action,
                      const FBridgeValidationContext &Context) {
  FBridgeValidateRequest Request;
  Request.Action = Action;
  Request.Context = Context;
  return Request;
}

} // namespace TypeFactory

namespace BridgeSlice {

struct FBridgeSliceState {
  rtk::EntityState<FDirectiveRuleSet> ActivePresets;
  rtk::EntityState<FDirectiveRuleSet> AvailableRulesets;
  TArray<FString> AvailablePresetIds;
  FValidationResult ValidationResult;
  bool bHasValidationResult{};
  FString Status;
  FString Error;
};

} // namespace BridgeSlice
