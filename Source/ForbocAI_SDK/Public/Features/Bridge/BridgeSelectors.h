#pragma once

#include "Core/fp.hpp"
#include "Features/Bridge/BridgeSlice.h"

namespace BridgeSelectors {

inline TArray<FDirectiveRuleSet>
selectActivePresets(const BridgeSlice::FBridgeSliceState &State) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectAll(
      State.ActivePresets);
}

inline func::Maybe<FDirectiveRuleSet>
selectActivePresetById(const BridgeSlice::FBridgeSliceState &State,
                       const FString &Id) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectById(
      State.ActivePresets, Id);
}

inline TArray<FDirectiveRuleSet>
selectAvailableRulesets(const BridgeSlice::FBridgeSliceState &State) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectAll(
      State.AvailableRulesets);
}

inline func::Maybe<FDirectiveRuleSet>
selectAvailableRulesetById(const BridgeSlice::FBridgeSliceState &State,
                           const FString &Id) {
  return BridgeSlice::bridgeRulesetAdapter().getSelectors().selectById(
      State.AvailableRulesets, Id);
}

inline const TArray<FString> &
selectAvailablePresetIds(const BridgeSlice::FBridgeSliceState &State) {
  return State.AvailablePresetIds;
}

inline func::Maybe<FValidationResult>
selectBridgeLastValidation(const BridgeSlice::FBridgeSliceState &State) {
  return State.bHasLastValidation
             ? func::just<FValidationResult>(State.LastValidation)
             : func::nothing<FValidationResult>();
}

inline bool
selectBridgeHasLastValidation(const BridgeSlice::FBridgeSliceState &State) {
  return State.bHasLastValidation;
}

inline const FString &
selectBridgeStatus(const BridgeSlice::FBridgeSliceState &State) {
  return State.Status;
}

inline const FString &
selectBridgeError(const BridgeSlice::FBridgeSliceState &State) {
  return State.Error;
}

} // namespace BridgeSelectors
