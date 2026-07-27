#pragma once

#include "CoreMinimal.h"

namespace APISlice::RulesConfiguration {

struct FDirectiveRuleFieldData {
  FString Id;
  FString Name;
  FString Conditions;
  FString Action;
  FString Reason;
  FString Target;
  FString Priority;
  FString ObservationPattern;
  FString PromptSuffix;
};

struct FDirectiveRulesetFieldData {
  FString Id;
  FString Rules;
  FString Template;
};

struct FConditionStructureData {
  int32 PairSize;
  int32 KeyIndex;
  int32 ValueIndex;
};

struct FRulesContractData {
  FDirectiveRuleFieldData Rule;
  FDirectiveRulesetFieldData Ruleset;
  FConditionStructureData Condition;
};

} // namespace APISlice::RulesConfiguration
