#pragma once

#include "Core/rtk.hpp"
#include "Features/Bridge/BridgeActions.h"
#include "Features/Bridge/BridgeAdapters.h"

namespace BridgeSlice {

struct FBridgeSliceState {
  rtk::EntityState<FDirectiveRuleSet> ActivePresets;
  rtk::EntityState<FDirectiveRuleSet> AvailableRulesets;
  TArray<FString> AvailablePresetIds;
  FValidationResult LastValidation;
  bool bHasLastValidation;
  FString Status;
  FString Error;

  FBridgeSliceState()
      : ActivePresets(bridgeRulesetAdapter().getInitialState()),
        AvailableRulesets(bridgeRulesetAdapter().getInitialState()),
        bHasLastValidation(false),
        Status(bridgeStatuses()[static_cast<int32>(EBridgeStatus::Idle)]) {}
};

inline rtk::Slice<FBridgeSliceState> createBridgeSlice() {
  return rtk::createSlice<FBridgeSliceState>(
      TEXT("bridge"), FBridgeSliceState(),
      [](rtk::ActionReducerMapBuilder<FBridgeSliceState> &Builder) {
        Builder.addCase(
            Actions::validationRequestedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              FBridgeSliceState Next = State;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Validating)];
              Next.Error.Empty();
              Next.LastValidation = FValidationResult();
              Next.bHasLastValidation = false;
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
              Next.LastValidation = Action.PayloadValue;
              Next.bHasLastValidation = true;
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
              Next.LastValidation = FValidationResult();
              Next.bHasLastValidation = false;
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
            Actions::rulesetRegisteredActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<FDirectiveRuleSet> &Action) {
              FBridgeSliceState Next = State;
              Next.AvailableRulesets = bridgeRulesetAdapter().upsertOne(
                  Next.AvailableRulesets, Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            Actions::rulesetDeletedActionCreator(),
            [](const FBridgeSliceState &State,
               const rtk::Action<FString> &Action) {
              FBridgeSliceState Next = State;
              Next.AvailableRulesets = bridgeRulesetAdapter().removeOne(
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
              Next.LastValidation = FValidationResult();
              Next.bHasLastValidation = false;
              Next.Status = bridgeStatuses()[
                  static_cast<int32>(EBridgeStatus::Idle)];
              Next.Error.Empty();
              return Next;
            });
      });
}

} // namespace BridgeSlice
