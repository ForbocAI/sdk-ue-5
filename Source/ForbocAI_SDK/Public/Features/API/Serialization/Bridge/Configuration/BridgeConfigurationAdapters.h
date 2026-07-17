#pragma once

#include "Features/API/Serialization/Bridge/Configuration/BridgeConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"

namespace JsonInterop::BridgeConfiguration {

/** User Story: As a Bridge contract maintainer, I need canonical wire fields loaded from authored JSON so validation and rule codecs share one schema. @fn inline FBridgeSerializationData readBridgeSerializationData() */
inline FBridgeSerializationData readBridgeSerializationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/bridge.json"));
  const TSharedRef<FJsonObject> Context =
      DataAdapters::ReadObjectField(Source, TEXT("context"));
  const TSharedRef<FJsonObject> ValidateRequest =
      DataAdapters::ReadObjectField(Source, TEXT("validateRequest"));
  const TSharedRef<FJsonObject> ValidationResult =
      DataAdapters::ReadObjectField(Source, TEXT("validationResult"));
  const TSharedRef<FJsonObject> Rule =
      DataAdapters::ReadObjectField(Source, TEXT("rule"));
  return {
      {DataAdapters::ReadStringField(Context, TEXT("npcState")),
       DataAdapters::ReadStringField(Context, TEXT("worldState")),
       DataAdapters::ReadStringField(Context, TEXT("constraints"))},
      {DataAdapters::ReadStringField(ValidateRequest, TEXT("action")),
       DataAdapters::ReadStringField(ValidateRequest, TEXT("context"))},
      {DataAdapters::ReadStringField(ValidationResult, TEXT("valid")),
       DataAdapters::ReadStringField(ValidationResult, TEXT("reason")),
       DataAdapters::ReadStringField(ValidationResult,
                                     TEXT("correctedAction"))},
      {DataAdapters::ReadStringField(Rule, TEXT("id")),
       DataAdapters::ReadStringField(Rule, TEXT("description")),
       DataAdapters::ReadStringField(Rule, TEXT("actions"))},
  };
}

/** User Story: As a Bridge codec consumer, I need one immutable authored schema per process so every endpoint uses identical wire fields. @fn inline const FBridgeSerializationData &bridgeSerializationData() */
inline const FBridgeSerializationData &bridgeSerializationData() {
  static const FBridgeSerializationData Data = readBridgeSerializationData();
  return Data;
}

} // namespace JsonInterop::BridgeConfiguration
