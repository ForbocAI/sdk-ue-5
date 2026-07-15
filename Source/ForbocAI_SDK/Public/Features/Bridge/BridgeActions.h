#pragma once

#include "Core/rtk.hpp"
#include "Features/Contracts/ContractsTypes.h"

namespace BridgeSlice::Actions {

inline const rtk::ActionCreatorWithoutPayload &
validationRequestedActionCreator() {
  static const rtk::ActionCreatorWithoutPayload Creator =
      rtk::createAction(TEXT("bridge/validationRequested"));
  return Creator;
}

inline const rtk::ActionCreator<FValidationResult> &
validationSucceededActionCreator() {
  static const rtk::ActionCreator<FValidationResult> Creator =
      rtk::createAction<FValidationResult>(TEXT("bridge/validationSucceeded"));
  return Creator;
}

inline const rtk::ActionCreator<FString> &validationFailedActionCreator() {
  static const rtk::ActionCreator<FString> Creator =
      rtk::createAction<FString>(TEXT("bridge/validationFailed"));
  return Creator;
}

inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> &
activePresetsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FDirectiveRuleSet>> Creator =
      rtk::createAction<TArray<FDirectiveRuleSet>>(
          TEXT("bridge/activePresetsReceived"));
  return Creator;
}

inline const rtk::ActionCreator<FDirectiveRuleSet> &
activePresetAddedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRuleSet> Creator =
      rtk::createAction<FDirectiveRuleSet>(TEXT("bridge/activePresetAdded"));
  return Creator;
}

inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> &
rulesetsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FDirectiveRuleSet>> Creator =
      rtk::createAction<TArray<FDirectiveRuleSet>>(
          TEXT("bridge/rulesetsReceived"));
  return Creator;
}

inline const rtk::ActionCreator<FDirectiveRuleSet> &
rulesetRegisteredActionCreator() {
  static const rtk::ActionCreator<FDirectiveRuleSet> Creator =
      rtk::createAction<FDirectiveRuleSet>(TEXT("bridge/rulesetRegistered"));
  return Creator;
}

inline const rtk::ActionCreator<FString> &rulesetDeletedActionCreator() {
  static const rtk::ActionCreator<FString> Creator =
      rtk::createAction<FString>(TEXT("bridge/rulesetDeleted"));
  return Creator;
}

inline const rtk::ActionCreator<TArray<FString>> &
presetIdsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FString>> Creator =
      rtk::createAction<TArray<FString>>(TEXT("bridge/presetIdsReceived"));
  return Creator;
}

inline const rtk::ActionCreatorWithoutPayload &
validationClearedActionCreator() {
  static const rtk::ActionCreatorWithoutPayload Creator =
      rtk::createAction(TEXT("bridge/validationCleared"));
  return Creator;
}

inline rtk::AnyAction validationRequested() {
  return validationRequestedActionCreator()();
}

inline rtk::AnyAction validationSucceeded(const FValidationResult &Result) {
  return validationSucceededActionCreator()(Result);
}

inline rtk::AnyAction validationFailed(const FString &Error) {
  return validationFailedActionCreator()(Error);
}

inline rtk::AnyAction
activePresetsReceived(const TArray<FDirectiveRuleSet> &Presets) {
  return activePresetsReceivedActionCreator()(Presets);
}

inline rtk::AnyAction activePresetAdded(const FDirectiveRuleSet &Preset) {
  return activePresetAddedActionCreator()(Preset);
}

inline rtk::AnyAction
rulesetsReceived(const TArray<FDirectiveRuleSet> &Rulesets) {
  return rulesetsReceivedActionCreator()(Rulesets);
}

inline rtk::AnyAction rulesetRegistered(const FDirectiveRuleSet &Ruleset) {
  return rulesetRegisteredActionCreator()(Ruleset);
}

inline rtk::AnyAction rulesetDeleted(const FString &RulesetId) {
  return rulesetDeletedActionCreator()(RulesetId);
}

inline rtk::AnyAction presetIdsReceived(const TArray<FString> &PresetIds) {
  return presetIdsReceivedActionCreator()(PresetIds);
}

inline rtk::AnyAction validationCleared() {
  return validationClearedActionCreator()();
}

} // namespace BridgeSlice::Actions
