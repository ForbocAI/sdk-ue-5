#pragma once

#include "Dom/JsonObject.h"

namespace Testing::API::Codec {

struct FSoulVerifyLabels {
  FString Decode;
  FString Valid;
  FString Reason;
};

struct FSoulVerifyFixture {
  FString ResponseJson;
  bool bExpectedValid;
  FString ExpectedReason;
  FSoulVerifyLabels Labels;
};

struct FBridgeRulesLabels {
  FString Decode;
  FString Count;
  FString Name;
  FString Description;
  FString ActionCount;
};

struct FBridgeRulesFixture {
  FString ResponseJson;
  int32 ExpectedCount;
  int32 FirstIndex;
  FString ExpectedName;
  FString ExpectedDescription;
  int32 ExpectedActionCount;
  FBridgeRulesLabels Labels;
};

struct FRulesetLabels {
  FString Decode;
  FString RulesetId;
  FString RuleCount;
  FString RuleId;
  FString RuleName;
  FString ConditionCount;
  FString ConditionKey;
  FString ConditionValue;
  FString Action;
  FString Reason;
  FString Target;
  FString Priority;
  FString ObservationPattern;
  FString PromptSuffix;
  FString Template;
};

struct FRulesetFixture {
  FString ResponseJson;
  FString ExpectedRulesetId;
  int32 ExpectedRuleCount;
  int32 FirstRuleIndex;
  FString ExpectedRuleId;
  FString ExpectedRuleName;
  int32 ExpectedConditionCount;
  int32 FirstConditionIndex;
  FString ExpectedConditionKey;
  FString ExpectedConditionValue;
  FString ExpectedAction;
  FString ExpectedReason;
  FString ExpectedTarget;
  int32 ExpectedPriority;
  FString ExpectedObservationPattern;
  FString ExpectedPromptSuffix;
  FString ExpectedTemplate;
  FRulesetLabels Labels;
};

struct FNullableProtocolLabels {
  FString Decode;
  FString Signature;
  FString Persona;
  FString RulesetId;
  FString RejectsMalformedStore;
  FString RejectsMalformedRecall;
};

struct FNullableProtocolFixture {
  FString ResponseJson;
  FString MalformedStoreResponseJson;
  FString MalformedRecallResponseJson;
  FNullableProtocolLabels Labels;
};

struct FBridgeValidationLabels {
  FString Decode;
  FString Valid;
  FString Reason;
  FString RejectsInvalidAction;
};

struct FBridgeValidationFixture {
  FString ResponseJson;
  FString InvalidActionResponseJson;
  bool bExpectedValid;
  FString ExpectedReason;
  FBridgeValidationLabels Labels;
};

struct FActionContractLabels {
  FString Decode;
  FString Type;
  FString Target;
  FString Reason;
  FString RejectsInternalNames;
};

struct FActionContractFixture {
  TSharedRef<FJsonObject> Input;
  TSharedRef<FJsonObject> InternalInput;
  FString ExpectedType;
  FString ExpectedTarget;
  FString ExpectedReason;
  FActionContractLabels Labels;
};

struct FIdentifyActorPayloadLabels {
  FString Payload;
  FString Type;
  FString NpcId;
  FString Persona;
  FString Data;
};

struct FIdentifyActorPayloadFixture {
  FString NpcId;
  FString Persona;
  FString DataJson;
  FString ExpectedType;
  FIdentifyActorPayloadLabels Labels;
};

struct FDecisionPayloadLabels {
  FString Payload;
  FString Type;
  FString Goal;
  FString ActionType;
  FString Target;
};

struct FDecisionPayloadFixture {
  FString Goal;
  FString ActionType;
  FString Target;
  FString ExpectedType;
  FDecisionPayloadLabels Labels;
};

struct FReasoningPayloadLabels {
  FString Payload;
  FString Type;
  FString ReasoningText;
  FString ResponseText;
};

struct FReasoningPayloadFixture {
  FString ReasoningText;
  FString ResponseText;
  FString ExpectedType;
  FReasoningPayloadLabels Labels;
};

struct FProcessTapePayloadLabels {
  FString Payload;
  FString Observation;
  FString Persona;
  FString ContextTime;
};

struct FProcessTapePayloadFixture {
  FString Observation;
  FString ContextJson;
  FString NpcStateJson;
  FString Persona;
  TArray<FString> ExpectedTraits;
  FString ExpectedContextTime;
  FProcessTapePayloadLabels Labels;
};

struct FCodecFixtures {
  FSoulVerifyFixture SoulVerify;
  FBridgeRulesFixture BridgeRules;
  FRulesetFixture Ruleset;
  FNullableProtocolFixture NullableProtocol;
  FBridgeValidationFixture BridgeValidation;
  FActionContractFixture ActionContract;
  FIdentifyActorPayloadFixture IdentifyActorPayload;
  FDecisionPayloadFixture DecisionPayload;
  FReasoningPayloadFixture ReasoningPayload;
  FProcessTapePayloadFixture ProcessTapePayload;
};

} // namespace Testing::API::Codec
