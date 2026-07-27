#pragma once

// The CLI/Bridge folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/Bridge/BridgeThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As an engine bridge consumer, I need validation failures represented explicitly so rejected transport cannot escape through the host event loop. @fn template <typename RuntimeState = FRuntimeState> inline func::Either<FString, FValidationResult> validateBridgePayloadEither(rtk::EnhancedStore<RuntimeState> &Store, const FAgentAction &Action, const FBridgeValidationContext &Context, const FString &NpcId = TEXT("")) */
template <typename RuntimeState = FRuntimeState>
inline func::Either<FString, FValidationResult>
validateBridgePayloadEither(rtk::EnhancedStore<RuntimeState> &Store,
                            const FAgentAction &Action,
                            const FBridgeValidationContext &Context,
                            const FString &NpcId = TEXT("")) {
  return AsyncAdapters::waitForEither(
      Store.dispatch(rtk::validateBridgeThunk(Action, Context, NpcId)));
}

/** User Story: As a features cli bridge consumer, I need to invoke validate bridge payload through a stable signature so the features cli bridge workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FValidationResult validateBridgePayload(rtk::EnhancedStore<RuntimeState> &Store, const FAgentAction &Action, const FBridgeValidationContext &Context, const FString &NpcId = TEXT("")) */
template <typename RuntimeState = FRuntimeState>
inline FValidationResult
validateBridgePayload(rtk::EnhancedStore<RuntimeState> &Store,
                      const FAgentAction &Action,
                      const FBridgeValidationContext &Context,
                      const FString &NpcId = TEXT("")) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::validateBridgeThunk(Action, Context, NpcId)));
}

/** User Story: As a features cli bridge consumer, I need to invoke get bridge rules through a stable signature so the features cli bridge workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FBridgeRule> getBridgeRules(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FBridgeRule>
getBridgeRules(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getBridgeRulesThunk()));
}

/** User Story: As a features cli bridge consumer, I need to invoke load bridge preset through a stable signature so the features cli bridge workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FDirectiveRuleSet loadBridgePreset(rtk::EnhancedStore<RuntimeState> &Store, const FString &PresetName) */
template <typename RuntimeState = FRuntimeState>
inline FDirectiveRuleSet
loadBridgePreset(rtk::EnhancedStore<RuntimeState> &Store,
                 const FString &PresetName) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::loadBridgePresetThunk(PresetName)));
}

/** User Story: As a features cli bridge consumer, I need to invoke list rulesets through a stable signature so the features cli bridge workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FDirectiveRuleSet> listRulesets(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FDirectiveRuleSet>
listRulesets(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listRulesetsThunk()));
}

/** User Story: As a features cli bridge consumer, I need to invoke list rule presets through a stable signature so the features cli bridge workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FString> listRulePresets(rtk::EnhancedStore<RuntimeState> &Store) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FString>
listRulePresets(rtk::EnhancedStore<RuntimeState> &Store) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listRulePresetsThunk()));
}

} // namespace Ops
