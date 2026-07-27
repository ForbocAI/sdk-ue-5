#pragma once

#include "Components/Bridge/BridgeTypes.h"
#include "Components/CLI/Bridge/CLIBridgeTypes.h"

namespace ForbocAI {
namespace CLI {
struct FCLIState;
namespace Bridge {

/** User Story: As a features cli bridge consumer, I need to invoke select cli bridge through a stable signature so the features cli bridge workflow remains explicit and composable. @fn const FCLIBridgeState &selectCliBridge(const FCLIState &State) */
const FCLIBridgeState &selectCliBridge(const FCLIState &State);

/** User Story: As a features cli bridge consumer, I need to invoke select bridge rule name through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString selectBridgeRuleName(const FBridgeRule &Rule, const FCLIBridgeState &State) */
inline FString selectBridgeRuleName(const FBridgeRule &Rule,
                                    const FCLIBridgeState &State) {
  return Rule.RuleName.IsEmpty() ? State.Messages.UnknownRule : Rule.RuleName;
}

/** User Story: As a features cli bridge consumer, I need to invoke select bridge rule description through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString selectBridgeRuleDescription(const FBridgeRule &Rule, const FCLIBridgeState &State) */
inline FString selectBridgeRuleDescription(const FBridgeRule &Rule,
                                           const FCLIBridgeState &State) {
  return Rule.RuleDescription.IsEmpty() ? State.Messages.NotAvailable
                                        : Rule.RuleDescription;
}

/** User Story: As a features cli bridge consumer, I need to invoke select bridge rule actions through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString selectBridgeRuleActions(const FBridgeRule &Rule, const FCLIBridgeState &State) */
inline FString selectBridgeRuleActions(const FBridgeRule &Rule,
                                       const FCLIBridgeState &State) {
  return FString::Join(Rule.RuleActionTypes, *State.Messages.ActionSeparator);
}

/** User Story: As a features cli bridge consumer, I need to invoke select ruleset id through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString selectRulesetId(const FDirectiveRuleSet &Ruleset, const FCLIBridgeState &State) */
inline FString selectRulesetId(const FDirectiveRuleSet &Ruleset,
                               const FCLIBridgeState &State) {
  return Ruleset.RulesetId.IsEmpty() ? State.Messages.UnknownRuleset
                                     : Ruleset.RulesetId;
}

} // namespace Bridge
} // namespace CLI
} // namespace ForbocAI
