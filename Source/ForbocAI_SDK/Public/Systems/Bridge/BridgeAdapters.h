#pragma once

#include "Core/rtk.hpp"
#include "Components/Contracts/ContractsTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace BridgeSlice {

/** User Story: As a features bridge consumer, I need to invoke bridge statuses through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const TArray<FString> &bridgeStatuses() */
inline const TArray<FString> &bridgeStatuses() {
  static const TArray<FString> Statuses = []() {
    const TArray<FString> Values =
        func::map_array<TSharedPtr<FJsonValue>, FString>(
            DataAdapters::ArraySource(TEXT("ForbocAI_SDK"),
                                      TEXT("Data/bridge/statuses.json"))
                .Root,
            [](const TSharedPtr<FJsonValue> &Value) {
              return Value->AsString();
            });
    check(Values.Num() == static_cast<int32>(EBridgeStatus::Count));
    return Values;
  }();
  return Statuses;
}

/** User Story: As a features bridge consumer, I need to invoke select bridge ruleset id through a stable signature so the features bridge workflow remains explicit and composable. @fn inline FString selectBridgeRulesetId(const FDirectiveRuleSet &Ruleset) */
inline FString selectBridgeRulesetId(const FDirectiveRuleSet &Ruleset) {
  return Ruleset.RulesetId;
}

/** User Story: As a features bridge consumer, I need to invoke bridge ruleset adapter through a stable signature so the features bridge workflow remains explicit and composable. @fn inline const rtk::EntityAdapter<FDirectiveRuleSet> &bridgeRulesetAdapter() */
inline const rtk::EntityAdapter<FDirectiveRuleSet> &bridgeRulesetAdapter() {
  static const rtk::EntityAdapter<FDirectiveRuleSet> Adapter =
      rtk::createEntityAdapter<FDirectiveRuleSet>(&selectBridgeRulesetId);
  return Adapter;
}

} // namespace BridgeSlice
