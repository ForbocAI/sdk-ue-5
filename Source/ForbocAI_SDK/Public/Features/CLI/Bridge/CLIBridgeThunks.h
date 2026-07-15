#pragma once

// The CLI/Bridge folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Bridge/BridgeThunks.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline FValidationResult
validateBridgePayload(rtk::EnhancedStore<RuntimeState> &Store,
                      const FAgentAction &Action,
                      const FBridgeValidationContext &Context,
                      const FString &NpcId = TEXT("")) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::validateBridgeThunk(Action, Context, NpcId)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FBridgeRule>
getBridgeRules(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getBridgeRulesThunk()));
}

template <typename RuntimeState = FRuntimeState>
inline FDirectiveRuleSet
loadBridgePreset(rtk::EnhancedStore<RuntimeState> &Store,
                 const FString &PresetName) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::loadBridgePresetThunk(PresetName)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FDirectiveRuleSet>
listRulesets(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listRulesetsThunk()));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FString>
listRulePresets(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listRulePresetsThunk()));
}

template <typename RuntimeState = FRuntimeState>
inline FDirectiveRuleSet
registerRuleset(rtk::EnhancedStore<RuntimeState> &Store,
                const FDirectiveRuleSet &Ruleset) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::registerRulesetThunk(Ruleset)));
}

template <typename RuntimeState = FRuntimeState>
inline rtk::FEmptyPayload
deleteRuleset(rtk::EnhancedStore<RuntimeState> &Store,
              const FString &RulesetId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::deleteRulesetThunk(RulesetId)));
}

} // namespace Ops
