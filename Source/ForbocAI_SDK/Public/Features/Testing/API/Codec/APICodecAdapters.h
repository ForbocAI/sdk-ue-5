#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

inline FSoulVerifyFixture
ReadSoulVerifyFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("soulVerify"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("response"))),
      DataAdapters::ReadBooleanField(Expected, TEXT("valid")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("valid")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
      },
  };
}

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
      DataAdapters::ReadStringField(Expected, TEXT("id")),
      DataAdapters::ReadStringField(Expected, TEXT("rulesetId")),
      DataAdapters::ReadNumberField(Expected, TEXT("ruleCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("firstRuleIndex")),
      DataAdapters::ReadStringField(Expected, TEXT("ruleName")),
      DataAdapters::ReadNumberField(Expected, TEXT("actionCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("firstActionIndex")),
      DataAdapters::ReadStringField(Expected, TEXT("action")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("id")),
          DataAdapters::ReadStringField(Labels, TEXT("rulesetId")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleCount")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleName")),
          DataAdapters::ReadStringField(Labels, TEXT("actionCount")),
          DataAdapters::ReadStringField(Labels, TEXT("action")),
      },
  };
}

inline FNullableProtocolFixture
ReadNullableProtocolFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("nullableProtocol"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("response"))),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("signature")),
          DataAdapters::ReadStringField(Labels, TEXT("persona")),
          DataAdapters::ReadStringField(Labels, TEXT("rulesetId")),
      },
  };
}

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
      DataAdapters::ReadBooleanField(Expected, TEXT("valid")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("valid")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
      },
  };
}

inline FActionAliasesFixture
ReadActionAliasesFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("actionAliases"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadObjectField(Object, TEXT("input")),
      DataAdapters::ReadStringField(Expected, TEXT("type")),
      DataAdapters::ReadStringField(Expected, TEXT("target")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("type")),
          DataAdapters::ReadStringField(Labels, TEXT("target")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
      },
  };
}

inline const FCodecFixtures &CodecFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/api/codec.json"));
  static const FCodecFixtures Fixtures = {
      ReadSoulVerifyFixture(Source),
      ReadBridgeRulesFixture(Source),
      ReadRulesetFixture(Source),
      ReadNullableProtocolFixture(Source),
      ReadBridgeValidationFixture(Source),
      ReadActionAliasesFixture(Source),
  };
  return Fixtures;
}

} // namespace Testing::API::Codec
