#pragma once

#include "Systems/API/Endpoints/Bridge/EndpointsBridgeAdapters.h"
#include "Systems/API/Endpoints/Rules/Configuration/RulesConfigurationAdapters.h"
#include "Systems/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the Rule cache owner, I need rulesets identified consistently across preset mutations and catalog queries. @fn inline rtk::FApiEndpointTag ruleTagAdapter(const FString &RulesetId) */
inline rtk::FApiEndpointTag ruleTagAdapter(const FString &RulesetId) {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.Rule, RulesetId);
}

/** User Story: As the Rule cache owner, I need one authored list identity for ruleset and preset catalogs. @fn inline rtk::FApiEndpointTag ruleListTagAdapter() */
inline rtk::FApiEndpointTag ruleListTagAdapter() {
  return Configuration::endpointListTag(
      Transport::transportQueryData().Tags.Rule);
}

} // namespace APISlice::Endpoints

namespace APISlice::Detail {

/** User Story: As a directive-rule consumer, I need API condition tuples validated against the authored tuple structure before entering feature state. @fn inline func::Maybe<FDirectiveRuleCondition> DecodeDirectiveRuleCondition(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FDirectiveRuleCondition>
DecodeDirectiveRuleCondition(const TSharedPtr<FJsonValue> &Value) {
  const RulesConfiguration::FRulesContractData &Data =
      RulesConfiguration::rulesContractData();
  return !Value.IsValid() || Value->Type != EJson::Array
             ? func::nothing<FDirectiveRuleCondition>()
             : [&]() {
                 const TArray<TSharedPtr<FJsonValue>> &Pair =
                     Value->AsArray();
                 const bool bValid =
                     Pair.Num() == Data.Condition.PairSize &&
                     Pair.IsValidIndex(Data.Condition.KeyIndex) &&
                     Pair.IsValidIndex(Data.Condition.ValueIndex) &&
                     Pair[Data.Condition.KeyIndex].IsValid() &&
                     Pair[Data.Condition.KeyIndex]->Type == EJson::String &&
                     Pair[Data.Condition.ValueIndex].IsValid() &&
                     Pair[Data.Condition.ValueIndex]->Type == EJson::String;
                 return !bValid
                            ? func::nothing<FDirectiveRuleCondition>()
                            : [&]() {
                                FDirectiveRuleCondition Condition;
                                Condition.Key =
                                    Pair[Data.Condition.KeyIndex]->AsString();
                                Condition.Value = Pair[Data.Condition.ValueIndex]
                                                      ->AsString();
                                return func::just(Condition);
                              }();
               }();
}

/** User Story: As a directive-rule consumer, I need every canonical API rule field decoded losslessly without collapsing it into a Bridge rule. @fn inline func::Maybe<FDirectiveRule> DecodeDirectiveRuleObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FDirectiveRule>
DecodeDirectiveRuleObject(const TSharedPtr<FJsonObject> &Object) {
  const RulesConfiguration::FRulesContractData &Data =
      RulesConfiguration::rulesContractData();
  const TArray<TSharedPtr<FJsonValue>> *Conditions = nullptr;
  double Priority{};
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Rule.Id) &&
      Object->HasTypedField<EJson::String>(Data.Rule.Name) &&
      Object->TryGetArrayField(Data.Rule.Conditions, Conditions) && Conditions &&
      Object->HasTypedField<EJson::String>(Data.Rule.Action) &&
      Object->HasTypedField<EJson::String>(Data.Rule.Reason) &&
      Object->TryGetNumberField(Data.Rule.Priority, Priority) &&
      FMath::IsFinite(Priority);
  return !bValid
             ? func::nothing<FDirectiveRule>()
             : func::match(
      func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                 FDirectiveRuleCondition>(
          *Conditions, DecodeDirectiveRuleCondition),
      [&Object, &Data, Priority](
          const TArray<FDirectiveRuleCondition> &DecodedConditions) {
        FDirectiveRule Rule;
        Rule.RuleId = Object->GetStringField(Data.Rule.Id);
        Rule.Name = Object->GetStringField(Data.Rule.Name);
        Rule.Conditions = DecodedConditions;
        Rule.Action = Object->GetStringField(Data.Rule.Action);
        Rule.Reason = Object->GetStringField(Data.Rule.Reason);
        Rule.Target = JsonInterop::OptionalStringFromField(
            Object, Data.Rule.Target);
        Rule.Priority = static_cast<int32>(Priority);
        Rule.ObservationPattern = JsonInterop::OptionalStringFromField(
            Object, Data.Rule.ObservationPattern);
        Rule.PromptSuffix = JsonInterop::OptionalStringFromField(
            Object, Data.Rule.PromptSuffix);
        return func::just(Rule);
      },
      []() { return func::nothing<FDirectiveRule>(); });
}

/** User Story: As a ruleset consumer, I need canonical ruleset objects traversed into full directive rules so malformed children reject the complete response. @fn inline func::Maybe<FDirectiveRuleSet> DecodeDirectiveRuleSetObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FDirectiveRuleSet>
DecodeDirectiveRuleSetObject(const TSharedPtr<FJsonObject> &Object) {
  const RulesConfiguration::FRulesContractData &Data =
      RulesConfiguration::rulesContractData();
  const TArray<TSharedPtr<FJsonValue>> *Rules = nullptr;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Ruleset.Id) &&
      Object->TryGetArrayField(Data.Ruleset.Rules, Rules) && Rules;
  return !bValid
             ? func::nothing<FDirectiveRuleSet>()
             : func::match(
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FDirectiveRule>(
          *Rules, [](const TSharedPtr<FJsonValue> &Value) {
            return Value.IsValid() && Value->Type == EJson::Object
                       ? DecodeDirectiveRuleObject(Value->AsObject())
                       : func::nothing<FDirectiveRule>();
          }),
      [&Object, &Data](const TArray<FDirectiveRule> &DecodedRules) {
        FDirectiveRuleSet Ruleset;
        Ruleset.RulesetId = Object->GetStringField(Data.Ruleset.Id);
        Ruleset.RulesetRules = DecodedRules;
        Ruleset.Template = JsonInterop::OptionalStringFromField(
            Object, Data.Ruleset.Template);
        return func::just(Ruleset);
      },
      []() { return func::nothing<FDirectiveRuleSet>(); });
}

/** User Story: As a ruleset endpoint, I need one canonical ruleset response decoded without identity aliases. @fn inline bool DecodeDirectiveRuleSetResponse(const FString &Json, FDirectiveRuleSet &Ruleset) */
inline bool DecodeDirectiveRuleSetResponse(const FString &Json,
                                           FDirectiveRuleSet &Ruleset) {
  TSharedPtr<FJsonObject> Root;
  return !JsonInterop::ParseJsonObject(Json, Root)
             ? false
             : func::match(
                   DecodeDirectiveRuleSetObject(Root),
                   [&Ruleset](const FDirectiveRuleSet &Decoded) {
                     Ruleset = Decoded;
                     return true;
                   },
                   []() { return false; });
}

/**
 * User Story: As a ruleset catalog, I need the complete response traversed so a malformed ruleset cannot produce partial success.
 * @fn inline bool DecodeDirectiveRuleSetListResponse( const FString &Json, TArray<FDirectiveRuleSet> &Rulesets)
 */
inline bool DecodeDirectiveRuleSetListResponse(
    const FString &Json, TArray<FDirectiveRuleSet> &Rulesets) {
  TArray<TSharedPtr<FJsonValue>> Values;
  return !JsonInterop::ParseJsonArray(Json, Values)
             ? false
             : func::match(
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FDirectiveRuleSet>(
          Values, [](const TSharedPtr<FJsonValue> &Value) {
            return Value.IsValid() && Value->Type == EJson::Object
                       ? DecodeDirectiveRuleSetObject(Value->AsObject())
                       : func::nothing<FDirectiveRuleSet>();
          }),
      [&Rulesets](const TArray<FDirectiveRuleSet> &Decoded) {
        Rulesets = Decoded;
        return true;
      },
      []() { return false; });
}

/** User Story: As a rule registration caller, I need domain condition pairs encoded into the API's canonical tuple representation. @fn inline TSharedPtr<FJsonValue> EncodeDirectiveRuleCondition(const FDirectiveRuleCondition &Condition) */
inline TSharedPtr<FJsonValue>
EncodeDirectiveRuleCondition(const FDirectiveRuleCondition &Condition) {
  return MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{
      MakeShared<FJsonValueString>(Condition.Key),
      MakeShared<FJsonValueString>(Condition.Value)});
}

/** User Story: As a rule registration caller, I need full directive rules encoded without dropping condition, priority, or prompt fields. @fn inline TSharedPtr<FJsonValue> EncodeDirectiveRule(const FDirectiveRule &Rule) */
inline TSharedPtr<FJsonValue>
EncodeDirectiveRule(const FDirectiveRule &Rule) {
  const RulesConfiguration::FRulesContractData &Data =
      RulesConfiguration::rulesContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Data.Rule.Id, Rule.RuleId);
  Object->SetStringField(Data.Rule.Name, Rule.Name);
  Object->SetArrayField(
      Data.Rule.Conditions,
      func::map_array<FDirectiveRuleCondition, TSharedPtr<FJsonValue>>(
          Rule.Conditions, EncodeDirectiveRuleCondition));
  Object->SetStringField(Data.Rule.Action, Rule.Action);
  Object->SetStringField(Data.Rule.Reason, Rule.Reason);
  JsonInterop::detail::SetIfNonEmpty(Object, Data.Rule.Target, Rule.Target);
  Object->SetNumberField(Data.Rule.Priority, Rule.Priority);
  JsonInterop::detail::SetIfNonEmpty(Object, Data.Rule.ObservationPattern,
                                     Rule.ObservationPattern);
  JsonInterop::detail::SetIfNonEmpty(Object, Data.Rule.PromptSuffix,
                                     Rule.PromptSuffix);
  return MakeShared<FJsonValueObject>(Object);
}

/** User Story: As a ruleset registration caller, I need the feature-domain ruleset encoded to the exact API wire contract. @fn inline FString EncodeDirectiveRuleSet(const FDirectiveRuleSet &Ruleset) */
inline FString EncodeDirectiveRuleSet(const FDirectiveRuleSet &Ruleset) {
  const RulesConfiguration::FRulesContractData &Data =
      RulesConfiguration::rulesContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Data.Ruleset.Id, Ruleset.RulesetId);
  Object->SetArrayField(
      Data.Ruleset.Rules,
      func::map_array<FDirectiveRule, TSharedPtr<FJsonValue>>(
          Ruleset.RulesetRules, EncodeDirectiveRule));
  JsonInterop::detail::SetIfNonEmpty(Object, Data.Ruleset.Template,
                                     Ruleset.Template);
  return ToJsonString(Object);
}

} // namespace APISlice::Detail
