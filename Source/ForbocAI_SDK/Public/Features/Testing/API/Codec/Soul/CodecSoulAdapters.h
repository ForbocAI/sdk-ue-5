#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/API/Codec/CodecTypes.h"

namespace Testing::API::Codec {

/** User Story: As a testing API codec consumer, I need the Soul verification fixture read from authored data so verification decoding remains deterministic. @fn inline FSoulVerifyFixture ReadSoulVerifyFixture(const DataAdapters::FSettingsSource &Source) */
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

} // namespace Testing::API::Codec
