#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/API/APIApi.h"
#include "Features/Bridge/BridgeSlice.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Errors/ErrorsAdapters.h"

namespace rtk {

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
                     APISlice::Endpoints::postBridgeValidate(
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
                     [Dispatch, PresetName](const FDirectiveRuleSet &Ruleset) {
                       FDirectiveRuleSet ActiveRuleset = Ruleset;
                       ActiveRuleset.Id = ActiveRuleset.Id.IsEmpty()
                                              ? PresetName
                                              : ActiveRuleset.Id;
                       Dispatch(BridgeSlice::Actions::activePresetAdded(
                           ActiveRuleset));
                       return detail::ResolveAsync(Ruleset);
                     });
  };
}

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

inline ThunkAction<FDirectiveRuleSet, FRuntimeState>
registerRulesetThunk(const FDirectiveRuleSet &Ruleset) {
  return [Ruleset](std::function<AnyAction(const AnyAction &)> Dispatch,
                   std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FDirectiveRuleSet> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<FDirectiveRuleSet>(ApiKeyError.value)
               : func::AsyncChain::then<FDirectiveRuleSet,
                                        FDirectiveRuleSet>(
                     APISlice::Endpoints::postRuleRegister(Ruleset)(Dispatch,
                                                                    GetState),
                     [Dispatch](const FDirectiveRuleSet &Registered) {
                       Dispatch(BridgeSlice::Actions::rulesetRegistered(
                           Registered));
                       return detail::ResolveAsync(Registered);
                     });
  };
}

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
deleteRulesetThunk(const FString &RulesetId) {
  return [RulesetId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
               ? detail::RejectAsync<rtk::FEmptyPayload>(ApiKeyError.value)
               : func::AsyncChain::then<rtk::FEmptyPayload,
                                        rtk::FEmptyPayload>(
                     APISlice::Endpoints::deleteRule(RulesetId)(Dispatch,
                                                                 GetState),
                     [Dispatch, RulesetId](
                         const rtk::FEmptyPayload &Payload) {
                       Dispatch(
                           BridgeSlice::Actions::rulesetDeleted(RulesetId));
                       return detail::ResolveAsync(Payload);
                     });
  };
}

} // namespace rtk
