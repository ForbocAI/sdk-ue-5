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

struct FAgentResponseProjectionLabels {
  FString Dialogue;
  FString Action;
  FString Thought;
  FString Reasoning;
  FString Prompt;
};

struct FAgentResponseProjectionFixture {
  FString Dialogue;
  FString ActionType;
  FString Goal;
  FString DecisionActionType;
  FString Target;
  FString ReasoningText;
  FString ResponseText;
  FString Prompt;
  FAgentResponseProjectionLabels Labels;
};

struct FProcessTapePayloadLabels {
  FString Payload;
  FString Observation;
  FString Persona;
  FString ContextTime;
  FString ThoughtProfile;
  FString LegalActions;
  FString VisitedActions;
  FString AvoidActions;
};

struct FProcessTapePayloadFixture {
  FString Observation;
  FString ContextJson;
  FString NpcStateJson;
  FString Persona;
  FString ThoughtProfile;
  TArray<FString> LegalActions;
  TArray<FString> VisitedActions;
  TArray<FString> AvoidActions;
  TArray<FString> ExpectedTraits;
  FString ContextTimeField;
  FString ExpectedContextTime;
  FString ExpectedThoughtProfile;
  int32 ExpectedLegalActionCount;
  int32 ExpectedVisitedActionCount;
  int32 ExpectedAvoidActionCount;
  FProcessTapePayloadLabels Labels;
};

struct FCodecFixtures {
  FSoulVerifyFixture SoulVerify;
  FBridgeRulesFixture BridgeRules;
  FRulesetFixture Ruleset;
  FNullableProtocolFixture NullableProtocol;
  FBridgeValidationFixture BridgeValidation;
  FActionContractFixture ActionContract;
  FAgentResponseProjectionFixture AgentResponseProjection;
  FProcessTapePayloadFixture ProcessTapePayload;
};

} // namespace Testing::API::Codec
