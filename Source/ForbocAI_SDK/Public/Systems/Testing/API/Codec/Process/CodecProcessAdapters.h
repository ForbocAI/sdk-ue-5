#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a protocol projection test, I need API-authored analytical results loaded from canonical JSON so no fixture semantics live in test source. @fn inline FAgentResponseProjectionFixture ReadAgentResponseProjectionFixture(const DataAdapters::FSettingsSource &Source) */
inline FAgentResponseProjectionFixture
ReadAgentResponseProjectionFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("agentResponseProjection"));
  const TSharedRef<FJsonObject> Input =
      DataAdapters::ReadObjectField(Object, TEXT("input"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Input, TEXT("dialogue")),
      DataAdapters::ReadStringField(Input, TEXT("actionType")),
      DataAdapters::ReadStringField(Input, TEXT("goal")),
      DataAdapters::ReadStringField(Input, TEXT("decisionActionType")),
      DataAdapters::ReadStringField(Input, TEXT("target")),
      DataAdapters::ReadStringField(Input, TEXT("reasoningText")),
      DataAdapters::ReadStringField(Input, TEXT("responseText")),
      DataAdapters::ReadStringField(Input, TEXT("prompt")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("dialogue")),
          DataAdapters::ReadStringField(Labels, TEXT("action")),
          DataAdapters::ReadStringField(Labels, TEXT("thought")),
          DataAdapters::ReadStringField(Labels, TEXT("reasoning")),
          DataAdapters::ReadStringField(Labels, TEXT("prompt")),
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
      DataAdapters::ReadStringField(Input, TEXT("thoughtProfile")),
      DataAdapters::ReadStringArrayField(Input, TEXT("legalActions")),
      DataAdapters::ReadStringArrayField(Input, TEXT("visitedActions")),
      DataAdapters::ReadStringArrayField(Input, TEXT("avoidActions")),
      DataAdapters::ReadStringArrayField(Expected, TEXT("traits")),
      DataAdapters::ReadStringField(Expected, TEXT("contextTimeField")),
      DataAdapters::ReadStringField(Expected, TEXT("contextTime")),
      DataAdapters::ReadStringField(Expected, TEXT("thoughtProfile")),
      DataAdapters::ReadNumberField(Expected, TEXT("legalActionCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("visitedActionCount")),
      DataAdapters::ReadNumberField(Expected, TEXT("avoidActionCount")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("payload")),
          DataAdapters::ReadStringField(Labels, TEXT("observation")),
          DataAdapters::ReadStringField(Labels, TEXT("persona")),
          DataAdapters::ReadStringField(Labels, TEXT("contextTime")),
          DataAdapters::ReadStringField(Labels, TEXT("thoughtProfile")),
          DataAdapters::ReadStringField(Labels, TEXT("legalActions")),
          DataAdapters::ReadStringField(Labels, TEXT("visitedActions")),
          DataAdapters::ReadStringField(Labels, TEXT("avoidActions")),
      },
  };
}

} // namespace Testing::API::Codec
