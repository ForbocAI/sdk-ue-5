#pragma once
/**
 * ᚱ bridge traffic should never hide who said what and when
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"
#include "Features/Bridge/BridgeActions.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicBridgeBridgeSliceHOptionalDomainId;
} } }

namespace BridgeSlice {

using namespace rtk;
using namespace func;

struct FBridgeSliceState {
  TArray<FDirectiveRuleSet> ActivePresets;
  TArray<FDirectiveRuleSet> AvailableRulesets;
  TArray<FString> AvailablePresetIds;
  FValidationResult LastValidation;
  bool bHasLastValidation;
  FString Status;
  FString Error;

  FBridgeSliceState() : bHasLastValidation(false), Status(TEXT("idle")) {}
};

/**
 * Builds the bridge slice reducer and extra cases.
 * User Story: As bridge runtime setup, I need one slice factory so validation,
 * rulesets, and presets share a single reducer definition.
 */
inline Slice<FBridgeSliceState> createBridgeSlice() {
  return rtk::createSlice<FBridgeSliceState>(
  TEXT("bridge"), FBridgeSliceState(),
  [](rtk::ActionReducerMapBuilder<FBridgeSliceState> &Builder) {
    Builder.addCase(Actions::bridgeValidationPendingActionCreator(),
      [](const FBridgeSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FBridgeSliceState {
                  FBridgeSliceState Next = State;
                  Next.Status = TEXT("validating");
                  Next.Error.Empty();
                  return Next;
                });
    Builder.addCase(Actions::bridgeValidationSuccessActionCreator(),
      [](const FBridgeSliceState &State,
                   const Action<FValidationResult> &Action) -> FBridgeSliceState {
                  FBridgeSliceState Next = State;
                  Next.Status = TEXT("idle");
                  Next.LastValidation = Action.PayloadValue;
                  Next.bHasLastValidation = true;
                  return Next;
                });
    Builder.addCase(Actions::bridgeValidationFailedActionCreator(),
      [](const FBridgeSliceState &State,
                             const Action<FString> &Action) -> FBridgeSliceState {
                            FBridgeSliceState Next = State;
                            Next.Status = TEXT("error");
                            Next.Error = Action.PayloadValue;
                            Next.LastValidation =
                                TypeFactory::Invalid(Action.PayloadValue);
                            Next.bHasLastValidation = true;
                            return Next;
                          });
    Builder.addCase(Actions::setActivePresetsActionCreator(),
      [](const FBridgeSliceState &State,
                             const Action<TArray<FDirectiveRuleSet>> &Action)
                              -> FBridgeSliceState {
                            FBridgeSliceState Next = State;
                            Next.ActivePresets = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::addActivePresetActionCreator(),
      [](const FBridgeSliceState &State,
                   const Action<FDirectiveRuleSet> &Action) -> FBridgeSliceState {
                  FBridgeSliceState Next = State;
                  const FString TargetId = Action.PayloadValue.Id.IsEmpty()
                                               ? Action.PayloadValue.RulesetId
                                               : Action.PayloadValue.Id;
                  return (Next.ActivePresets.IndexOfByPredicate(
                              [&TargetId](const FDirectiveRuleSet &Preset) {
                                const FString ExistingId =
                                    Preset.Id.IsEmpty() ? Preset.RulesetId
                                                        : Preset.Id;
                                return ExistingId == TargetId;
                              }) == INDEX_NONE
                              ? (Next.ActivePresets.Add(Action.PayloadValue), void())
                              : void(),
                          Next);
                });
    Builder.addCase(Actions::setAvailableRulesetsActionCreator(),
      [](const FBridgeSliceState &State,
                             const Action<TArray<FDirectiveRuleSet>> &Action)
                              -> FBridgeSliceState {
                            FBridgeSliceState Next = State;
                            Next.AvailableRulesets = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::setAvailablePresetIdsActionCreator(),
      [](const FBridgeSliceState &State,
                   const Action<TArray<FString>> &Action) -> FBridgeSliceState {
                  FBridgeSliceState Next = State;
                  Next.AvailablePresetIds = Action.PayloadValue;
                  return Next;
                });
    Builder.addCase(Actions::clearBridgeValidationActionCreator(),
      [](const FBridgeSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FBridgeSliceState {
                  FBridgeSliceState Next = State;
                  Next.LastValidation = FValidationResult();
                  Next.bHasLastValidation = false;
                  Next.Status = TEXT("idle");
                  Next.Error.Empty();
                  return Next;
                });
  });
}

inline FString activePresetId(const FDirectiveRuleSet &Preset) {
  return Preset.Id.IsEmpty() ? Preset.RulesetId : Preset.Id;
}

inline func::Maybe<FDirectiveRuleSet>
selectActivePresetByIdRecursive(const TArray<FDirectiveRuleSet> &Presets,
                                const FString &Id, int32 Index) {
  return Index >= Presets.Num()
             ? func::nothing<FDirectiveRuleSet>()
             : activePresetId(Presets[Index]) == Id
                   ? func::just(Presets[Index])
                   : selectActivePresetByIdRecursive(Presets, Id, Index + 1);
}

inline TArray<FDirectiveRuleSet>
selectActivePresets(const FBridgeSliceState &State) {
  return State.ActivePresets;
}

inline func::Maybe<FDirectiveRuleSet>
selectActivePresetById(const FBridgeSliceState &State, const FString &Id) {
  return selectActivePresetByIdRecursive(State.ActivePresets, Id, 0);
}

} // namespace BridgeSlice
