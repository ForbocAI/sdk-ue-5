#pragma once

#include "Systems/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Systems/API/Endpoints/NPC/Directive/NPCDirectiveAdapters.h"
#include "Systems/API/Transport/Configuration/TransportConfigurationAdapters.h"

namespace APISlice::Endpoints {

/** User Story: As the Bridge cache owner, I need validation and rule entities identified consistently across queries and mutations. @fn inline rtk::FApiEndpointTag bridgeTagAdapter(const FString &Id = FString()) */
inline rtk::FApiEndpointTag bridgeTagAdapter(const FString &Id = FString()) {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.Bridge, Id);
}

/** User Story: As the Bridge cache owner, I need one authored list identity for the rule catalog. @fn inline rtk::FApiEndpointTag bridgeListTagAdapter() */
inline rtk::FApiEndpointTag bridgeListTagAdapter() {
  return Configuration::endpointListTag(
      Transport::transportQueryData().Tags.Bridge);
}

} // namespace APISlice::Endpoints

namespace APISlice::Detail {

/** User Story: As a Bridge rules consumer, I need one canonical bridge-rule object converted into the SDK domain without accepting directive-rule aliases. @fn inline func::Maybe<FBridgeRule> DecodeBridgeRuleObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FBridgeRule>
DecodeBridgeRuleObject(const TSharedPtr<FJsonObject> &Object) {
  const JsonInterop::BridgeConfiguration::FBridgeSerializationData &Data =
      JsonInterop::BridgeConfiguration::bridgeSerializationData();
  const TArray<TSharedPtr<FJsonValue>> *RawActions = nullptr;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Rule.Id) &&
      Object->HasTypedField<EJson::String>(Data.Rule.Description) &&
      Object->TryGetArrayField(Data.Rule.Actions, RawActions) && RawActions;
  return !bValid
             ? func::nothing<FBridgeRule>()
             : [&]() {
  const func::Maybe<TArray<FString>> Actions =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
          *RawActions, [](const TSharedPtr<FJsonValue> &Value) {
            return Value.IsValid() && Value->Type == EJson::String
                       ? func::just(Value->AsString())
                       : func::nothing<FString>();
          });
  return func::match(
      Actions,
      [&Object, &Data](const TArray<FString> &Values) {
        FBridgeRule Rule;
        Rule.RuleName = Object->GetStringField(Data.Rule.Id);
        Rule.RuleDescription = Object->GetStringField(Data.Rule.Description);
        Rule.RuleActionTypes = Values;
        return func::just(Rule);
      },
      []() { return func::nothing<FBridgeRule>(); });
               }();
}

/** User Story: As a Bridge rules consumer, I need the entire canonical response traversed so one malformed rule fails the query rather than disappearing. @fn inline bool DecodeBridgeRulesResponse(const FString &Json, TArray<FBridgeRule> &Rules) */
inline bool DecodeBridgeRulesResponse(const FString &Json,
                                      TArray<FBridgeRule> &Rules) {
  TArray<TSharedPtr<FJsonValue>> Values;
  return !JsonInterop::ParseJsonArray(Json, Values)
             ? false
             : func::match(
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FBridgeRule>(
          Values, [](const TSharedPtr<FJsonValue> &Value) {
            return Value.IsValid() && Value->Type == EJson::Object
                       ? DecodeBridgeRuleObject(Value->AsObject())
                       : func::nothing<FBridgeRule>();
          }),
      [&Rules](const TArray<FBridgeRule> &Decoded) {
        Rules = Decoded;
        return true;
      },
      []() { return false; });
}

} // namespace APISlice::Detail
