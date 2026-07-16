#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need the bridge-rules fixture read from authored data so rule-list decoding remains deterministic. @fn inline FBridgeRulesFixture ReadBridgeRulesFixture(const DataAdapters::FSettingsSource &Source) */
inline FBridgeRulesFixture
ReadBridgeRulesFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("bridgeRules"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeArray(
          DataAdapters::ReadArrayField(Object, TEXT("response"))),
      DataAdapters::ReadNumberField(Expected, TEXT("count")),
      DataAdapters::ReadNumberField(Expected, TEXT("firstIndex")),
      DataAdapters::ReadStringField(Expected, TEXT("name")),
      DataAdapters::ReadStringField(Expected, TEXT("description")),
      DataAdapters::ReadNumberField(Expected, TEXT("actionCount")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("count")),
          DataAdapters::ReadStringField(Labels, TEXT("name")),
          DataAdapters::ReadStringField(Labels, TEXT("description")),
          DataAdapters::ReadStringField(Labels, TEXT("actionCount")),
      },
  };
}

/** User Story: As a testing API codec consumer, I need the bridge-validation fixture read from authored data so malformed actions cannot pass as valid. @fn inline FBridgeValidationFixture ReadBridgeValidationFixture(const DataAdapters::FSettingsSource &Source) */
inline FBridgeValidationFixture
ReadBridgeValidationFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("bridgeValidation"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("response"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("invalidActionResponse"))),
      DataAdapters::ReadBooleanField(Expected, TEXT("valid")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("valid")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
          DataAdapters::ReadStringField(Labels, TEXT("rejectsInvalidAction")),
      },
  };
}

} // namespace Testing::API::Codec
