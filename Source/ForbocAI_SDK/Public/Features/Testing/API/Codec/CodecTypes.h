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
  FString Id;
  FString RulesetId;
  FString RuleCount;
  FString RuleName;
  FString ActionCount;
  FString Action;
};

struct FRulesetFixture {
  FString ResponseJson;
  FString ExpectedId;
  FString ExpectedRulesetId;
  int32 ExpectedRuleCount;
  int32 FirstRuleIndex;
  FString ExpectedRuleName;
  int32 ExpectedActionCount;
  int32 FirstActionIndex;
  FString ExpectedAction;
  FRulesetLabels Labels;
};

struct FNullableProtocolLabels {
  FString Decode;
  FString Signature;
  FString Persona;
  FString RulesetId;
};

struct FNullableProtocolFixture {
  FString ResponseJson;
  FNullableProtocolLabels Labels;
};

struct FBridgeValidationLabels {
  FString Decode;
  FString Valid;
  FString Reason;
};

struct FBridgeValidationFixture {
  FString ResponseJson;
  bool bExpectedValid;
  FString ExpectedReason;
  FBridgeValidationLabels Labels;
};

struct FActionAliasesLabels {
  FString Type;
  FString Target;
  FString Reason;
};

struct FActionAliasesFixture {
  TSharedRef<FJsonObject> Input;
  FString ExpectedType;
  FString ExpectedTarget;
  FString ExpectedReason;
  FActionAliasesLabels Labels;
};

struct FIdentifyActorPayloadLabels {
  FString Payload;
  FString Type;
  FString NpcId;
  FString Persona;
  FString Health;
};

struct FIdentifyActorPayloadFixture {
  FString NpcId;
  FString Persona;
  FString DataJson;
  FString ExpectedType;
  int32 ExpectedHealth;
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
  FActionAliasesFixture ActionAliases;
  FIdentifyActorPayloadFixture IdentifyActorPayload;
  FDecisionPayloadFixture DecisionPayload;
  FReasoningPayloadFixture ReasoningPayload;
  FProcessTapePayloadFixture ProcessTapePayload;
};

} // namespace Testing::API::Codec
