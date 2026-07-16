#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/API/APIApi.h"
#include "Features/Bridge/BridgeSlice.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Errors/ErrorsAdapters.h"

namespace rtk {

/** User Story: As a features bridge consumer, I need to invoke validate bridge thunk through a stable signature so the features bridge workflow remains explicit and composable. @fn inline ThunkAction<FValidationResult, FRuntimeState> validateBridgeThunk(const FAgentAction &Action, const FBridgeValidationContext &Context, const FString &NpcId = TEXT("")) */
inline ThunkAction<FValidationResult, FRuntimeState>
validateBridgeThunk(const FAgentAction &Action,
                    const FBridgeValidationContext &Context,
                    const FString &NpcId = TEXT("")) {
  return [Action, Context, NpcId](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FValidationResult> {
    Dispatch(BridgeSlice::Actions::validationRequested());
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? (Dispatch(BridgeSlice::Actions::validationFailed(
                      ApiKeyError.value)),
                  detail::RejectAsync<FValidationResult>(ApiKeyError.value))
               : func::AsyncChain::then<FValidationResult,
                                        FValidationResult>(
                     APISlice::Endpoints::getBridgeValidation(
                         NpcId,
                         TypeFactory::BridgeValidateRequest(Action, Context))(
                         Dispatch, GetState),
                     [Dispatch](const FValidationResult &Result) {
                       Dispatch(
                           BridgeSlice::Actions::validationSucceeded(Result));
                       return detail::ResolveAsync(Result);
                     })
                     .catch_([Dispatch](std::string Error) {
                       Dispatch(BridgeSlice::Actions::validationFailed(
                           FString(UTF8_TO_TCHAR(Error.c_str()))));
                     });
  };
}

/** User Story: As a features bridge consumer, I need to invoke load bridge preset thunk through a stable signature so the features bridge workflow remains explicit and composable. @fn inline ThunkAction<FDirectiveRuleSet, FRuntimeState> loadBridgePresetThunk(const FString &PresetName) */
inline ThunkAction<FDirectiveRuleSet, FRuntimeState>
loadBridgePresetThunk(const FString &PresetName) {
  return [PresetName](std::function<AnyAction(const AnyAction &)> Dispatch,
                      std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FDirectiveRuleSet> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<FDirectiveRuleSet>(ApiKeyError.value)
               : func::AsyncChain::then<FDirectiveRuleSet,
                                        FDirectiveRuleSet>(
                     APISlice::Endpoints::postBridgePreset(PresetName)(
                         Dispatch, GetState),
                     [Dispatch](const FDirectiveRuleSet &Ruleset) {
                       Dispatch(
                           BridgeSlice::Actions::activePresetAdded(Ruleset));
                       return detail::ResolveAsync(Ruleset);
                     });
  };
}

/** User Story: As a features bridge consumer, I need to invoke get bridge rules thunk through a stable signature so the features bridge workflow remains explicit and composable. @fn inline ThunkAction<TArray<FBridgeRule>, FRuntimeState> getBridgeRulesThunk() */
inline ThunkAction<TArray<FBridgeRule>, FRuntimeState> getBridgeRulesThunk() {
  return [](std::function<AnyAction(const AnyAction &)> Dispatch,
            std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FBridgeRule>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<TArray<FBridgeRule>>(ApiKeyError.value)
               : APISlice::Endpoints::getBridgeRules()(Dispatch, GetState);
  };
}

/** User Story: As a features bridge consumer, I need to invoke list rulesets thunk through a stable signature so the features bridge workflow remains explicit and composable. @fn inline ThunkAction<TArray<FDirectiveRuleSet>, FRuntimeState> listRulesetsThunk() */
inline ThunkAction<TArray<FDirectiveRuleSet>, FRuntimeState>
listRulesetsThunk() {
  return [](std::function<AnyAction(const AnyAction &)> Dispatch,
            std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FDirectiveRuleSet>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<TArray<FDirectiveRuleSet>>(
                     ApiKeyError.value)
               : func::AsyncChain::then<TArray<FDirectiveRuleSet>,
                                        TArray<FDirectiveRuleSet>>(
                     APISlice::Endpoints::getRulesets()(Dispatch, GetState),
                     [Dispatch](const TArray<FDirectiveRuleSet> &Rulesets) {
                       Dispatch(
                           BridgeSlice::Actions::rulesetsReceived(Rulesets));
                       return detail::ResolveAsync(Rulesets);
                     });
  };
}

/** User Story: As a features bridge consumer, I need to invoke list rule presets thunk through a stable signature so the features bridge workflow remains explicit and composable. @fn inline ThunkAction<TArray<FString>, FRuntimeState> listRulePresetsThunk() */
inline ThunkAction<TArray<FString>, FRuntimeState> listRulePresetsThunk() {
  return [](std::function<AnyAction(const AnyAction &)> Dispatch,
            std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FString>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<TArray<FString>>(ApiKeyError.value)
               : func::AsyncChain::then<TArray<FString>, TArray<FString>>(
                     APISlice::Endpoints::getRulePresets()(Dispatch,
                                                           GetState),
                     [Dispatch](const TArray<FString> &PresetIds) {
                       Dispatch(BridgeSlice::Actions::presetIdsReceived(
                           PresetIds));
                       return detail::ResolveAsync(PresetIds);
                     });
  };
}

} // namespace rtk
