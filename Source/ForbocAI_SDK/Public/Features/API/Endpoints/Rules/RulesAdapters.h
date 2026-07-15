#pragma once

#include "Features/API/Endpoints/NPC/Directive/NPCDirectiveAdapters.h"

namespace APISlice::Detail {

/**
 * Reads a string array field from a JSON object.
 * User Story: As codec helpers, I need a reusable array reader so repeated
 * string-list decoding stays consistent across bridge and ruleset payloads.
 */
inline TArray<FString>
DecodeStringArrayField(const TSharedPtr<FJsonObject> &Object,
                       const FString &FieldName) {
  TArray<FString> Values;
  const TArray<TSharedPtr<FJsonValue>> *RawValues = nullptr;
  return (!Object.IsValid() ||
          !Object->TryGetArrayField(FieldName, RawValues) || !RawValues)
             ? Values
             : (Values.Reserve(RawValues->Num()),
                detail::ExtractStringValuesRecursive(*RawValues, Values, 0),
                Values);
}

/**
 * Decodes a bridge-rule object into a typed bridge rule.
 * User Story: As bridge rule management, I need rule-object decoding so mixed
 * API field shapes still produce one normalized bridge-rule value.
 */
inline bool DecodeBridgeRuleObject(const TSharedPtr<FJsonObject> &Object,
                                   FBridgeRule &Rule) {
  return !Object.IsValid()
             ? false
             : [&]() -> bool {
                 const FString RuleName1 =
                     JsonInterop::OptionalStringFromField(Object,
                                                          TEXT("ruleName"));
                 const FString RuleName2 =
                     !RuleName1.IsEmpty()
                         ? RuleName1
                         : JsonInterop::OptionalStringFromField(
                               Object, TEXT("brRuleId"));
                 Rule.RuleName =
                     !RuleName2.IsEmpty()
                         ? RuleName2
                         : JsonInterop::OptionalStringFromField(
                               Object, TEXT("ruleId"));

                 const FString RuleDesc1 =
                     JsonInterop::OptionalStringFromField(
                         Object, TEXT("ruleDescription"));
                 Rule.RuleDescription =
                     !RuleDesc1.IsEmpty()
                         ? RuleDesc1
                         : JsonInterop::OptionalStringFromField(
                               Object, TEXT("ruleReason"));

                 const TArray<FString> Actions1 =
                     DecodeStringArrayField(Object, TEXT("ruleActionTypes"));
                 const TArray<FString> Actions2 =
                     Actions1.Num() > 0
                         ? Actions1
                         : DecodeStringArrayField(Object,
                                                  TEXT("affectedActions"));
                 Rule.RuleActionTypes =
                     Actions2.Num() > 0
                         ? Actions2
                         : [&]() -> TArray<FString> {
                             const FString SingleAction =
                                 JsonInterop::OptionalStringFromField(
                                     Object, TEXT("ruleAction"));
                             TArray<FString> Result;
                             !SingleAction.IsEmpty()
                                 ? (Result.Add(SingleAction), void())
                                 : void();
                             return Result;
                           }();

                 return true;
               }();
}

/**
 * Recursive bridge-rule extraction definition.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
namespace detail {
inline void ExtractBridgeRulesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FBridgeRule> &Out,
    int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? [&]() {
                 FBridgeRule Rule;
                 DecodeBridgeRuleObject(Source[Index]->AsObject(), Rule)
                     ? (Out.Add(Rule), void())
                     : void();
               }()
             : void(),
         ExtractBridgeRulesRecursive(Source, Out, Index + 1), void())
      : void();
}
} // namespace detail

/**
 * Decodes a directive ruleset object into a typed ruleset value.
 * User Story: As directive rule management, I need ruleset-object decoding so
 * nested rule arrays become a usable runtime ruleset structure.
 */
inline bool DecodeDirectiveRuleSetObject(const TSharedPtr<FJsonObject> &Object,
                                         FDirectiveRuleSet &Ruleset) {
  return !Object.IsValid()
             ? false
             : (Ruleset.Id = JsonInterop::OptionalStringFromField(
                    Object, TEXT("id")),
                Ruleset.RulesetId = JsonInterop::OptionalStringFromField(
                    Object, TEXT("rulesetId")),
                Ruleset.Id.IsEmpty()
                    ? (Ruleset.Id = Ruleset.RulesetId, void())
                    : void(),
                Ruleset.RulesetRules.Empty(),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *RuleValues = nullptr;
                  (Object->TryGetArrayField(TEXT("rulesetRules"), RuleValues) &&
                   RuleValues)
                      ? (Ruleset.RulesetRules.Reserve(RuleValues->Num()),
                         detail::ExtractBridgeRulesRecursive(
                             *RuleValues, Ruleset.RulesetRules, 0),
                         void())
                      : void();
                }(),
                true);
}

/**
 * Recursive directive ruleset extraction definition.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
namespace detail {
inline void ExtractDirectiveRuleSetsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FDirectiveRuleSet> &Out, int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? [&]() {
                 FDirectiveRuleSet Ruleset;
                 DecodeDirectiveRuleSetObject(Source[Index]->AsObject(), Ruleset)
                     ? (Out.Add(Ruleset), void())
                     : void();
               }()
             : void(),
         ExtractDirectiveRuleSetsRecursive(Source, Out, Index + 1), void())
      : void();
}
} // namespace detail

/**
 * Decodes a bridge-rules response into a typed rule collection.
 * User Story: As bridge rule management, I need list-response decoding so API
 * rule inventories can be consumed without handwritten JSON walking.
 */
inline bool DecodeBridgeRulesResponse(const FString &Json,
                                      TArray<FBridgeRule> &Rules) {
  TArray<TSharedPtr<FJsonValue>> Values;
  return !JsonInterop::ParseJsonArray(Json, Values)
             ? false
             : (Rules.Empty(), Rules.Reserve(Values.Num()),
                detail::ExtractBridgeRulesRecursive(Values, Rules, 0), true);
}

/**
 * Decodes a directive-ruleset response into a single ruleset value.
 * User Story: As directive rule management, I need single-ruleset decoding so
 * details views and edits can load one ruleset cleanly.
 */
inline bool DecodeDirectiveRuleSetResponse(const FString &Json,
                                           FDirectiveRuleSet &Ruleset) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : DecodeDirectiveRuleSetObject(Root, Ruleset);
}

/**
 * Decodes a directive-ruleset list response into typed rulesets.
 * User Story: As directive rule management, I need list-response decoding so
 * ruleset indexes can hydrate strongly typed runtime collections.
 */
inline bool
DecodeDirectiveRuleSetListResponse(const FString &Json,
                                   TArray<FDirectiveRuleSet> &Rulesets) {
  TArray<TSharedPtr<FJsonValue>> Values;
  return !JsonInterop::ParseJsonArray(Json, Values)
             ? false
             : (Rulesets.Empty(), Rulesets.Reserve(Values.Num()),
                detail::ExtractDirectiveRuleSetsRecursive(Values, Rulesets, 0),
                true);
}

} // namespace APISlice::Detail
