#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need nullable protocol fixtures read from authored data so malformed optional payloads fail atomically. @fn inline FNullableProtocolFixture ReadNullableProtocolFixture(const DataAdapters::FSettingsSource &Source) */
inline FNullableProtocolFixture
ReadNullableProtocolFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("nullableProtocol"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("response"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("malformedStoreResponse"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Object, TEXT("malformedRecallResponse"))),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("signature")),
          DataAdapters::ReadStringField(Labels, TEXT("persona")),
          DataAdapters::ReadStringField(Labels, TEXT("rulesetId")),
          DataAdapters::ReadStringField(Labels, TEXT("rejectsMalformedStore")),
          DataAdapters::ReadStringField(Labels, TEXT("rejectsMalformedRecall")),
      },
  };
}

/** User Story: As a testing API codec consumer, I need the canonical action fixture read from authored data so transport decoding is verified without compatibility aliases. @fn inline FActionContractFixture ReadActionContractFixture(const DataAdapters::FSettingsSource &Source) */
inline FActionContractFixture
ReadActionContractFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("actionContract"));
  const TSharedRef<FJsonObject> Expected =
      DataAdapters::ReadObjectField(Object, TEXT("expected"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadObjectField(Object, TEXT("input")),
      DataAdapters::ReadObjectField(Object, TEXT("internalInput")),
      DataAdapters::ReadStringField(Expected, TEXT("type")),
      DataAdapters::ReadStringField(Expected, TEXT("target")),
      DataAdapters::ReadStringField(Expected, TEXT("reason")),
      {
          DataAdapters::ReadStringField(Labels, TEXT("decode")),
          DataAdapters::ReadStringField(Labels, TEXT("type")),
          DataAdapters::ReadStringField(Labels, TEXT("target")),
          DataAdapters::ReadStringField(Labels, TEXT("reason")),
          DataAdapters::ReadStringField(Labels, TEXT("rejectsInternalNames")),
      },
  };
}

} // namespace Testing::API::Codec
