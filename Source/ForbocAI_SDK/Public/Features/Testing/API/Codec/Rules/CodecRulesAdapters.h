#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need the ruleset fixture read from authored data so every canonical rule field is verified. @fn inline FRulesetFixture ReadRulesetFixture(const DataAdapters::FSettingsSource &Source) */
inline FRulesetFixture
ReadRulesetFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("ruleset"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("response"))),
      DataAdapters::ReadStringField(Expected, TEXT("rulesetId")),
      DataAdapters::ReadNumberField(Expected, TEXT("ruleCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("firstRuleIndex")),
      DataAdapters::ReadStringField(Expected, TEXT("ruleId")),
      DataAdapters::ReadStringField(Expected, TEXT("ruleName")),
      DataAdapters::ReadNumberField(Expected, TEXT("conditionCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("firstConditionIndex")),
      DataAdapters::ReadStringField(Expected, TEXT("conditionKey")),
      DataAdapters::ReadStringField(Expected, TEXT("conditionValue")),
      DataAdapters::ReadStringField(Expected, TEXT("action")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      DataAdapters::ReadStringField(Expected, TEXT("target")),
      DataAdapters::ReadNumberField(Expected, TEXT("priority")),
      DataAdapters::ReadStringField(Expected, TEXT("observationPattern")),
      DataAdapters::ReadStringField(Expected, TEXT("promptSuffix")),
      DataAdapters::ReadStringField(Expected, TEXT("template")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("rulesetId")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleCount")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleId")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleName")),
          DataAdapters::ReadStringField(Labels, TEXT("conditionCount")),
          DataAdapters::ReadStringField(Labels, TEXT("conditionKey")),
          DataAdapters::ReadStringField(Labels, TEXT("conditionValue")),
          DataAdapters::ReadStringField(Labels, TEXT("action")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
          DataAdapters::ReadStringField(Labels, TEXT("target")),
          DataAdapters::ReadStringField(Labels, TEXT("priority")),
          DataAdapters::ReadStringField(Labels, TEXT("observationPattern")),
          DataAdapters::ReadStringField(Labels, TEXT("promptSuffix")),
          DataAdapters::ReadStringField(Labels, TEXT("template")),
      },
  };
}

} // namespace Testing::API::Codec
