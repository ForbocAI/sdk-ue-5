#pragma once

#include "Core/rtk.hpp"
#include "Features/Bridge/BridgeActions.h"
#include "Features/Bridge/BridgeAdapters.h"

namespace BridgeSlice {

/**
 * User Story: As bridge initialization, I need adapter-owned entity state and
 * authored status values composed at the slice boundary.
 * @fn inline FBridgeSliceState createBridgeInitialState()
 */
inline FBridgeSliceState createBridgeInitialState() {
  FBridgeSliceState State;
  State.ActivePresets = bridgeRulesetAdapter().getInitialState();
  State.AvailableRulesets = bridgeRulesetAdapter().getInitialState();
  State.Status =
      bridgeStatuses()[static_cast<int32>(EBridgeStatus::Idle)];
  return State;
}

/** User Story: As a features bridge consumer, I need to invoke create bridge slice through a stable signature so the features bridge workflow remains explicit and composable. @fn inline rtk::Slice<FBridgeSliceState> createBridgeSlice() */
inline rtk::Slice<FBridgeSliceState> createBridgeSlice() {
  return rtk::createSlice<FBridgeSliceState>(
      TEXT("bridge"), createBridgeInitialState(),
      [](rtk::ActionReducerMapBuilder<FBridgeSliceState> &Builder) {
        Builder.addCase(
            Actions::validationRequestedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FBridgeSliceState Next = State;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Validating)];
              Next.Error.Empty();
              Next.ValidationResult = FValidationResult();
              Next.bHasValidationResult = false;
              return Next;
            });
        Builder.addCase(
            Actions::validationSucceededActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<FValidationResult> &Action) {
              FBridgeSliceState Next = State;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Idle)];
              Next.Error.Empty();
              Next.ValidationResult = Action.PayloadValue;
              Next.bHasValidationResult = true;
              return Next;
            });
        Builder.addCase(
            Actions::validationFailedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<FString> &Action) {
              FBridgeSliceState Next = State;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Error)];
              Next.Error = Action.PayloadValue;
              Next.ValidationResult = FValidationResult();
              Next.bHasValidationResult = false;
              return Next;
            });
        Builder.addCase(
            Actions::activePresetsReceivedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<TArray<FDirectiveRuleSet>> &Action) {
              FBridgeSliceState Next = State;
              Next.ActivePresets = bridgeRulesetAdapter().setAll(
                  Next.ActivePresets, Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            Actions::activePresetAddedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<FDirectiveRuleSet> &Action) {
              FBridgeSliceState Next = State;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Idle)];
              Next.ActivePresets = bridgeRulesetAdapter().upsertOne(
                  Next.ActivePresets, Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            Actions::rulesetsReceivedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<TArray<FDirectiveRuleSet>> &Action) {
              FBridgeSliceState Next = State;
              Next.AvailableRulesets = bridgeRulesetAdapter().setAll(
                  Next.AvailableRulesets, Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            Actions::presetIdsReceivedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<TArray<FString>> &Action) {
              FBridgeSliceState Next = State;
              Next.AvailablePresetIds = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::validationClearedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FBridgeSliceState Next = State;
              Next.ValidationResult = FValidationResult();
              Next.bHasValidationResult = false;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Idle)];
              Next.Error.Empty();
              return Next;
            });
      });
}

} // namespace BridgeSlice
