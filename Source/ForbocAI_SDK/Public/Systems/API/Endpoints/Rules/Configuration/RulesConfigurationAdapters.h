#pragma once

#include "Components/API/Endpoints/Rules/Configuration/RulesConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace APISlice::RulesConfiguration {

/** User Story: As a Rules contract maintainer, I need directive wire fields and tuple structure loaded from authored JSON so TS and UE decode the same API schema. @fn inline FRulesContractData readRulesContractData() */
inline FRulesContractData readRulesContractData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/rules.json"));
  const TSharedRef<FJsonObject> Rule =
      DataAdapters::ReadObjectField(Source, TEXT("rule"));
  const TSharedRef<FJsonObject> Ruleset =
      DataAdapters::ReadObjectField(Source, TEXT("ruleset"));
  const TSharedRef<FJsonObject> Condition =
      DataAdapters::ReadObjectField(Source, TEXT("condition"));
  return {
      {DataAdapters::ReadStringField(Rule, TEXT("id")),
       DataAdapters::ReadStringField(Rule, TEXT("name")),
       DataAdapters::ReadStringField(Rule, TEXT("conditions")),
       DataAdapters::ReadStringField(Rule, TEXT("action")),
       DataAdapters::ReadStringField(Rule, TEXT("reason")),
       DataAdapters::ReadStringField(Rule, TEXT("target")),
       DataAdapters::ReadStringField(Rule, TEXT("priority")),
       DataAdapters::ReadStringField(Rule, TEXT("observationPattern")),
       DataAdapters::ReadStringField(Rule, TEXT("promptSuffix"))},
      {DataAdapters::ReadStringField(Ruleset, TEXT("id")),
       DataAdapters::ReadStringField(Ruleset, TEXT("rules")),
       DataAdapters::ReadStringField(Ruleset, TEXT("template"))},
      {DataAdapters::ReadNumberField(Condition, TEXT("pairSize")),
       DataAdapters::ReadNumberField(Condition, TEXT("keyIndex")),
       DataAdapters::ReadNumberField(Condition, TEXT("valueIndex"))},
  };
}

/** User Story: As a Rules codec consumer, I need one immutable authored contract per process so every endpoint uses identical directive fields. @fn inline const FRulesContractData &rulesContractData() */
inline const FRulesContractData &rulesContractData() {
  static const FRulesContractData Data = readRulesContractData();
  return Data;
}

} // namespace APISlice::RulesConfiguration
