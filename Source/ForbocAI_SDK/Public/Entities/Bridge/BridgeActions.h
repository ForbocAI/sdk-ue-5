#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Components/Contracts/ContractsTypes.h"

namespace BridgeSlice::Actions {

/** User Story: As a features bridge consumer, I need to invoke validation requested action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreatorWithoutPayload & validationRequestedActionCreator() */
inline const rtk::ActionCreatorWithoutPayload &
validationRequestedActionCreator() {
  static const rtk::ActionCreatorWithoutPayload Creator =
      rtk::createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCDC07C9D3C91));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke validation succeeded action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FValidationResult> & validationSucceededActionCreator() */
inline const rtk::ActionCreator<FValidationResult> &
validationSucceededActionCreator() {
  static const rtk::ActionCreator<FValidationResult> Creator =
      rtk::createAction<FValidationResult>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE083DDBFB586));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke validation failed action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FString> &validationFailedActionCreator() */
inline const rtk::ActionCreator<FString> &validationFailedActionCreator() {
  static const rtk::ActionCreator<FString> Creator =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0C3301845C76));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke active presets received action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> & activePresetsReceivedActionCreator() */
inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> &
activePresetsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FDirectiveRuleSet>> Creator =
      rtk::createAction<TArray<FDirectiveRuleSet>>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV164209C30C6C));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke active preset added action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FDirectiveRuleSet> & activePresetAddedActionCreator() */
inline const rtk::ActionCreator<FDirectiveRuleSet> &
activePresetAddedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRuleSet> Creator =
      rtk::createAction<FDirectiveRuleSet>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE84B31BB6399));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke rulesets received action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> & rulesetsReceivedActionCreator() */
inline const rtk::ActionCreator<TArray<FDirectiveRuleSet>> &
rulesetsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FDirectiveRuleSet>> Creator =
      rtk::createAction<TArray<FDirectiveRuleSet>>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7319BF013F0B));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke preset ids received action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreator<TArray<FString>> & presetIdsReceivedActionCreator() */
inline const rtk::ActionCreator<TArray<FString>> &
presetIdsReceivedActionCreator() {
  static const rtk::ActionCreator<TArray<FString>> Creator =
      rtk::createAction<TArray<FString>>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEC5C5A97AC9D));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke validation cleared action creator through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::ActionCreatorWithoutPayload & validationClearedActionCreator() */
inline const rtk::ActionCreatorWithoutPayload &
validationClearedActionCreator() {
  static const rtk::ActionCreatorWithoutPayload Creator =
      rtk::createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV954183F70CF1));
  return Creator;
}

/** User Story: As a features bridge consumer, I need to invoke validation requested through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction validationRequested() */
inline rtk::AnyAction validationRequested() {
  return validationRequestedActionCreator()();
}

/** User Story: As a features bridge consumer, I need to invoke validation succeeded through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction validationSucceeded(const FValidationResult &Result) */
inline rtk::AnyAction validationSucceeded(const FValidationResult &Result) {
  return validationSucceededActionCreator()(Result);
}

/** User Story: As a features bridge consumer, I need to invoke validation failed through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction validationFailed(const FString &Error) */
inline rtk::AnyAction validationFailed(const FString &Error) {
  return validationFailedActionCreator()(Error);
}

/** User Story: As a features bridge consumer, I need to invoke active presets received through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction activePresetsReceived(const TArray<FDirectiveRuleSet> &Presets) */
inline rtk::AnyAction
activePresetsReceived(const TArray<FDirectiveRuleSet> &Presets) {
  return activePresetsReceivedActionCreator()(Presets);
}

/** User Story: As a features bridge consumer, I need to invoke active preset added through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction activePresetAdded(const FDirectiveRuleSet &Preset) */
inline rtk::AnyAction activePresetAdded(const FDirectiveRuleSet &Preset) {
  return activePresetAddedActionCreator()(Preset);
}

/** User Story: As a features bridge consumer, I need to invoke rulesets received through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction rulesetsReceived(const TArray<FDirectiveRuleSet> &Rulesets) */
inline rtk::AnyAction
rulesetsReceived(const TArray<FDirectiveRuleSet> &Rulesets) {
  return rulesetsReceivedActionCreator()(Rulesets);
}

/** User Story: As a features bridge consumer, I need to invoke preset ids received through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction presetIdsReceived(const TArray<FString> &PresetIds) */
inline rtk::AnyAction presetIdsReceived(const TArray<FString> &PresetIds) {
  return presetIdsReceivedActionCreator()(PresetIds);
}

/** User Story: As a features bridge consumer, I need to invoke validation cleared through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::AnyAction validationCleared() */
inline rtk::AnyAction validationCleared() {
  return validationClearedActionCreator()();
}

} // namespace BridgeSlice::Actions
