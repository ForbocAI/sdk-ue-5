#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need the identify-actor payload fixture read from authored data so actor serialization remains canonical. @fn inline FIdentifyActorPayloadFixture ReadIdentifyActorPayloadFixture(const DataAdapters::FSettingsSource &Source) */
inline FIdentifyActorPayloadFixture
ReadIdentifyActorPayloadFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("identifyActorPayload"));
  const TSharedRef<FJsonObject> Input =
      DataAdapters::ReadObjectField(Object, TEXT("input"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Input, TEXT("npcId")),
      DataAdapters::ReadStringField(Input, TEXT("persona")),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Input, TEXT("data"))),
      DataAdapters::ReadStringField(Expected, TEXT("type")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("payload")),
          DataAdapters::ReadStringField(Labels, TEXT("type")),
          DataAdapters::ReadStringField(Labels, TEXT("npcId")),
          DataAdapters::ReadStringField(Labels, TEXT("persona")),
          DataAdapters::ReadStringField(Labels, TEXT("data")),
      },
  };
}

/** User Story: As a testing API codec consumer, I need the decision payload fixture read from authored data so decision serialization remains canonical. @fn inline FDecisionPayloadFixture ReadDecisionPayloadFixture(const DataAdapters::FSettingsSource &Source) */
inline FDecisionPayloadFixture
ReadDecisionPayloadFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("decisionPayload"));
  const TSharedRef<FJsonObject> Input =
      DataAdapters::ReadObjectField(Object, TEXT("input"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Input, TEXT("goal")),
      DataAdapters::ReadStringField(Input, TEXT("actionType")),
      DataAdapters::ReadStringField(Input, TEXT("target")),
      DataAdapters::ReadStringField(Expected, TEXT("type")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("payload")),
          DataAdapters::ReadStringField(Labels, TEXT("type")),
          DataAdapters::ReadStringField(Labels, TEXT("goal")),
          DataAdapters::ReadStringField(Labels, TEXT("actionType")),
          DataAdapters::ReadStringField(Labels, TEXT("target")),
      },
  };
}

/** User Story: As a testing API codec consumer, I need the reasoning payload fixture read from authored data so reasoning serialization remains canonical. @fn inline FReasoningPayloadFixture ReadReasoningPayloadFixture(const DataAdapters::FSettingsSource &Source) */
inline FReasoningPayloadFixture
ReadReasoningPayloadFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("reasoningPayload"));
  const TSharedRef<FJsonObject> Input =
      DataAdapters::ReadObjectField(Object, TEXT("input"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Input, TEXT("reasoningText")),
      DataAdapters::ReadStringField(Input, TEXT("responseText")),
      DataAdapters::ReadStringField(Expected, TEXT("type")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("payload")),
          DataAdapters::ReadStringField(Labels, TEXT("type")),
          DataAdapters::ReadStringField(Labels, TEXT("reasoningText")),
          DataAdapters::ReadStringField(Labels, TEXT("responseText")),
      },
  };
}

/** User Story: As a testing API codec consumer, I need the process-tape payload fixture read from authored data so nested tape serialization remains canonical. @fn inline FProcessTapePayloadFixture ReadProcessTapePayloadFixture(const DataAdapters::FSettingsSource &Source) */
inline FProcessTapePayloadFixture
ReadProcessTapePayloadFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("processTapePayload"));
  const TSharedRef<FJsonObject> Input =
      DataAdapters::ReadObjectField(Object, TEXT("input"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Input, TEXT("observation")),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Input, TEXT("context"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Input, TEXT("npcState"))),
      DataAdapters::ReadStringField(Input, TEXT("persona")),
      DataAdapters::ReadStringArrayField(Expected, TEXT("traits")),
      DataAdapters::ReadStringField(Expected, TEXT("contextTime")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("payload")),
          DataAdapters::ReadStringField(Labels, TEXT("observation")),
          DataAdapters::ReadStringField(Labels, TEXT("persona")),
          DataAdapters::ReadStringField(Labels, TEXT("contextTime")),
      },
  };
}

} // namespace Testing::API::Codec
