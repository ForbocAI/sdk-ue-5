#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace CLI {
namespace Bridge {

struct FCLIBridgeLimits {
  int32 EmptyItemCount;
  int32 RequiredArgumentCount;
};

struct FCLIBridgeSyntax {
  FString InlineObjectPrefix;
};

struct FCLIBridgeFields {
  FString Action;
  FString Distance;
  FString NpcId;
  FString Context;
  FString NpcState;
  FString WorldState;
  FString Constraints;
};

struct FCLIBridgeMessages {
  FString EmptyObject;
  FString FetchingRules;
  FString NoRules;
  FString UnknownRule;
  FString NotAvailable;
  FString RuleName;
  FString Description;
  FString Actions;
  FString ActionSeparator;
  FString ValidateUsage;
  FString Validation;
  FString Pass;
  FString Fail;
  FString Reason;
  FString CorrectedAction;
  FString ValidationDone;
  FString RulesCount;
  FString RulesListed;
  FString PresetUsage;
  FString PresetLoaded;
  FString PresetDone;
  FString RulesetsCount;
  FString RulesetsListed;
  FString NoRulesets;
  FString RulesetItem;
  FString UnknownRuleset;
  FString PresetsCount;
  FString PresetsListed;
  FString NoPresets;
  FString PresetItem;
};

struct FCLIBridgeState {
  FCLIBridgeLimits Limits;
  FCLIBridgeSyntax Syntax;
  FCLIBridgeFields Fields;
  FCLIBridgeMessages Messages;
};

} // namespace Bridge
} // namespace CLI
} // namespace ForbocAI
