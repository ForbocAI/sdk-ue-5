#pragma once

#include "Core/fp.hpp"
#include "Features/Bridge/BridgeSlice.h"

namespace BridgeSelectors {

/** User Story: As a features bridge consumer, I need to invoke select active presets through a stable signature so the features bridge workflow remains explicit and composable. @fn inline TArray<FDirectiveRuleSet> selectActivePresets(const BridgeSlice::FBridgeSliceState &State) */
inline TArray<FDirectiveRuleSet>
selectActivePresets(const BridgeSlice::FBridgeSliceState &State) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectAll(
      State.ActivePresets);
}

/** User Story: As a features bridge consumer, I need to invoke select active preset by id through a stable signature so the features bridge workflow remains explicit and composable. @fn inline func::Maybe<FDirectiveRuleSet> selectActivePresetById(const BridgeSlice::FBridgeSliceState &State, const FString &Id) */
inline func::Maybe<FDirectiveRuleSet>
selectActivePresetById(const BridgeSlice::FBridgeSliceState &State,
                       const FString &Id) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectById(
      State.ActivePresets, Id);
}

/** User Story: As a features bridge consumer, I need to invoke select available rulesets through a stable signature so the features bridge workflow remains explicit and composable. @fn inline TArray<FDirectiveRuleSet> selectAvailableRulesets(const BridgeSlice::FBridgeSliceState &State) */
inline TArray<FDirectiveRuleSet>
selectAvailableRulesets(const BridgeSlice::FBridgeSliceState &State) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectAll(
      State.AvailableRulesets);
}

/** User Story: As a features bridge consumer, I need to invoke select available ruleset by id through a stable signature so the features bridge workflow remains explicit and composable. @fn inline func::Maybe<FDirectiveRuleSet> selectAvailableRulesetById(const BridgeSlice::FBridgeSliceState &State, const FString &Id) */
inline func::Maybe<FDirectiveRuleSet>
selectAvailableRulesetById(const BridgeSlice::FBridgeSliceState &State,
                           const FString &Id) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectById(
      State.AvailableRulesets, Id);
}

/** User Story: As a features bridge consumer, I need to invoke select available preset ids through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const TArray<FString> & selectAvailablePresetIds(const BridgeSlice::FBridgeSliceState &State) */
inline const TArray<FString> &
selectAvailablePresetIds(const BridgeSlice::FBridgeSliceState &State) {
  return State.AvailablePresetIds;
}

/** User Story: As a features bridge consumer, I need to invoke select bridge last validation through a stable signature so the features bridge workflow remains explicit and composable. @fn inline func::Maybe<FValidationResult> selectBridgeValidationResult(const BridgeSlice::FBridgeSliceState &State) */
inline func::Maybe<FValidationResult>
selectBridgeValidationResult(const BridgeSlice::FBridgeSliceState &State) {
  return State.bHasValidationResult
             ? func::just<FValidationResult>(State.ValidationResult)
             : func::nothing<FValidationResult>();
}

/** User Story: As a features bridge consumer, I need to invoke select bridge has last validation through a stable signature so the features bridge workflow remains explicit and composable. @fn inline bool selectBridgeHasValidationResult(const BridgeSlice::FBridgeSliceState &State) */
inline bool
selectBridgeHasValidationResult(const BridgeSlice::FBridgeSliceState &State) {
  return State.bHasValidationResult;
}

/** User Story: As a features bridge consumer, I need to invoke select bridge status through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const FString & selectBridgeStatus(const BridgeSlice::FBridgeSliceState &State) */
inline const FString &
selectBridgeStatus(const BridgeSlice::FBridgeSliceState &State) {
  return State.Status;
}

/** User Story: As a features bridge consumer, I need to invoke select bridge error through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const FString & selectBridgeError(const BridgeSlice::FBridgeSliceState &State) */
inline const FString &
selectBridgeError(const BridgeSlice::FBridgeSliceState &State) {
  return State.Error;
}

} // namespace BridgeSelectors
