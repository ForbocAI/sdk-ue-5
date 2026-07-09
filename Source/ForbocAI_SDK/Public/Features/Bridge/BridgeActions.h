#pragma once
/**
 * Bridge action creators own bridge event construction.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"

namespace BridgeSlice {

using namespace rtk;
using namespace func;

namespace Actions {

/**
 * Returns the memoized action creator for pending bridge validation.
 * User Story: As bridge validation flows, I need a cached pending action
 * creator so every caller dispatches the same start signal.
 */
inline const ActionCreatorWithoutPayload &bridgeValidationPendingActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("bridge/validationPending"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for successful bridge validation.
 * User Story: As bridge validation success handling, I need a cached action
 * creator so validated results enter state through one contract.
 */
inline const ActionCreator<FValidationResult> &
bridgeValidationSuccessActionCreator() {
  static const ActionCreator<FValidationResult> ActionCreator =
      createAction<FValidationResult>(TEXT("bridge/validationSuccess"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for failed bridge validation.
 * User Story: As bridge validation error handling, I need a cached failure
 * action creator so validation problems are surfaced consistently.
 */
inline const ActionCreator<FString> &bridgeValidationFailedActionCreator() {
  static const ActionCreator<FString> ActionCreator =
      createAction<FString>(TEXT("bridge/validationFailed"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for replacing active presets.
 * User Story: As bridge preset management, I need a cached action creator so
 * active preset lists can be replaced through one reducer contract.
 */
inline const ActionCreator<TArray<FDirectiveRuleSet>> &
setActivePresetsActionCreator() {
  static const ActionCreator<TArray<FDirectiveRuleSet>> ActionCreator =
      createAction<TArray<FDirectiveRuleSet>>(TEXT("bridge/setActivePresets"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for adding one active preset.
 * User Story: As bridge preset management, I need a cached action creator so
 * one preset can be appended without rebuilding the full list.
 */
inline const ActionCreator<FDirectiveRuleSet> &addActivePresetActionCreator() {
  static const ActionCreator<FDirectiveRuleSet> ActionCreator =
      createAction<FDirectiveRuleSet>(TEXT("bridge/addActivePreset"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for replacing available rulesets.
 * User Story: As bridge rules catalog updates, I need a cached action creator
 * so fetched rulesets can replace stale catalog data consistently.
 */
inline const ActionCreator<TArray<FDirectiveRuleSet>> &
setAvailableRulesetsActionCreator() {
  static const ActionCreator<TArray<FDirectiveRuleSet>> ActionCreator =
      createAction<TArray<FDirectiveRuleSet>>(
          TEXT("bridge/setAvailableRulesets"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for replacing preset ids.
 * User Story: As preset discovery flows, I need a cached action creator so
 * available preset ids are updated through a shared contract.
 */
inline const ActionCreator<TArray<FString>> &
setAvailablePresetIdsActionCreator() {
  static const ActionCreator<TArray<FString>> ActionCreator =
      createAction<TArray<FString>>(TEXT("bridge/setAvailablePresetIds"));
  return ActionCreator;
}

/**
 * Returns the memoized action creator for clearing validation state.
 * User Story: As bridge cleanup flows, I need a cached clear action creator so
 * stale validation results can be reset predictably.
 */
inline const ActionCreatorWithoutPayload &clearBridgeValidationActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("bridge/clearBridgeValidation"));
  return ActionCreator;
}

/**
 * Builds the action that marks bridge validation as pending.
 * User Story: As bridge status tracking, I need a helper that dispatches the
 * pending action without manual action construction.
 */
inline AnyAction bridgeValidationPending() {
  return bridgeValidationPendingActionCreator()();
}

/**
 * Builds the action that records a successful validation result.
 * User Story: As bridge success handling, I need a helper so valid bridge
 * results can be stored with a single call.
 */
inline AnyAction bridgeValidationSuccess(const FValidationResult &Result) {
  return bridgeValidationSuccessActionCreator()(Result);
}

/**
 * Builds the action that records a validation failure message.
 * User Story: As bridge error handling, I need a helper so validation failures
 * can be dispatched without hand-assembling payloads.
 */
inline AnyAction bridgeValidationFailure(const FString &Error) {
  return bridgeValidationFailedActionCreator()(Error);
}

/**
 * Builds the action that replaces the active bridge presets.
 * User Story: As preset synchronization, I need a helper so the current active
 * preset list can be refreshed in one dispatch.
 */
inline AnyAction setActivePresets(const TArray<FDirectiveRuleSet> &Presets) {
  return setActivePresetsActionCreator()(Presets);
}

/**
 * Builds the action that appends an active bridge preset.
 * User Story: As preset editing flows, I need a helper so one preset can be
 * added to the active set without custom action wiring.
 */
inline AnyAction addActivePreset(const FDirectiveRuleSet &Preset) {
  return addActivePresetActionCreator()(Preset);
}

/**
 * Builds the action that replaces the available ruleset list.
 * User Story: As rules catalog refresh flows, I need a helper so fetched
 * rulesets replace the current catalog through one action.
 */
inline AnyAction
setAvailableRulesets(const TArray<FDirectiveRuleSet> &Rulesets) {
  return setAvailableRulesetsActionCreator()(Rulesets);
}

/**
 * Builds the action that replaces the available preset id list.
 * User Story: As preset discovery flows, I need a helper so available preset
 * ids can be refreshed through one action.
 */
inline AnyAction setAvailablePresetIds(const TArray<FString> &PresetIds) {
  return setAvailablePresetIdsActionCreator()(PresetIds);
}

/**
 * Builds the action that clears bridge validation state.
 * User Story: As bridge cleanup flows, I need a helper so old validation
 * results and errors can be cleared before the next run.
 */
inline AnyAction clearBridgeValidation() {
  return clearBridgeValidationActionCreator()();
}

} // namespace Actions

} // namespace BridgeSlice
