#pragma once

#include "Core/rtk.hpp"
#include "Features/Contracts/ContractsTypes.h"
#include "Features/Data/DataAdapters.h"

namespace BridgeSlice {

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

inline FString selectBridgeRulesetId(const FDirectiveRuleSet &Ruleset) {
  return Ruleset.Id.IsEmpty() ? Ruleset.RulesetId : Ruleset.Id;
}

inline const rtk::EntityAdapter<FDirectiveRuleSet> &bridgeRulesetAdapter() {
  static const rtk::EntityAdapter<FDirectiveRuleSet> Adapter =
      rtk::createEntityAdapter<FDirectiveRuleSet>(&selectBridgeRulesetId);
  return Adapter;
}

} // namespace BridgeSlice
