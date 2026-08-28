#pragma once

#include "CoreMinimal.h"

namespace APISlice::NPCProcessConfiguration {

struct FResponseFields {
  FString Instruction;
  FString Tape;
};

struct FRequestFields {
  FString Tape;
  FString LastResult;
};

struct FTapeFields {
  FString Observation;
  FString Context;
  FString NpcState;
  FString StructuredPersona;
  FString ThoughtProfile;
  FString Actor;
  FString Memories;
  FString DecisionIntent;
  FString ReasoningOutput;
  FString Prompt;
  FString Constraints;
  FString GeneratedOutput;
  FString RulesetId;
  FString VectorQueried;
  FString LegalActions;
  FString VisitedActions;
  FString AvoidActions;
};

struct FActorFields {
  FString Id;
  FString StructuredPersona;
  FString Data;
};

struct FDecisionIntentFields {
  FString Goal;
  FString ActionType;
  FString Target;
  FString Metadata;
};

struct FReasoningOutputFields {
  FString ReasoningText;
  FString ResponseText;
};

struct FPromptConstraintsFields {
  FString MaxTokens;
  FString Temperature;
  FString Stop;
  FString RepeatPenalty;
  FString Seed;
};

struct FInstructionFields {
  FString Type;
  FString Query;
  FString Limit;
  FString Threshold;
};

struct FInstructionTypes {
  FString QueryVector;
  FString Finalize;
};

struct FResultTypes {
  FString QueryVector;
};

struct FFinalizeFields {
  FString Valid;
  FString Signature;
  FString MemoryStore;
  FString StateTransform;
  FString Action;
  FString Dialogue;
  FString RuleAudit;
};

struct FRuleAuditFields {
  FString Preset;
  FString AppliedRules;
};

struct FRuleVerdictFields {
  FString RuleId;
  FString Verdict;
  FString Reason;
};

struct FVerdictValues {
  FString Pass;
  FString Blocked;
};

struct FProcessContractData {
  FResponseFields Response;
  FRequestFields Request;
  FTapeFields Tape;
  FActorFields Actor;
  FDecisionIntentFields DecisionIntent;
  FReasoningOutputFields ReasoningOutput;
  FPromptConstraintsFields PromptConstraints;
  FInstructionFields Instruction;
  FInstructionTypes InstructionTypes;
  FResultTypes ResultTypes;
  FFinalizeFields Finalize;
  FRuleAuditFields RuleAudit;
  FRuleVerdictFields RuleVerdict;
  FVerdictValues Verdicts;
};

} // namespace APISlice::NPCProcessConfiguration
