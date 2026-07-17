#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/API/CacheTags/CacheTagsTypes.h"

namespace Testing::API::CacheTags {

/**
 * User Story: As an API cache-tag test, I need authored expected tags decoded through one typed adapter.
 * @fn inline rtk::FApiEndpointTag ReadTag( const TSharedRef<FJsonObject> &Tags, const FString &Field, bool HasId = true)
 */
inline rtk::FApiEndpointTag ReadTag(
    const TSharedRef<FJsonObject> &Tags, const FString &Field,
    bool HasId = true) {
  const TSharedRef<FJsonObject> Tag =
      DataAdapters::ReadObjectField(Tags, Field);
  return {DataAdapters::ReadStringField(Tag, TEXT("type")),
          HasId ? DataAdapters::ReadStringField(Tag, TEXT("id"))
                : FString()};
}

/**
 * User Story: As an API cache-tag test, I need every label loaded from authored data so assertion intent remains portable.
 * @fn inline FCacheTagLabels ReadLabels( const DataAdapters::FSettingsSource &Source)
 */
inline FCacheTagLabels ReadLabels(
    const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {DataAdapters::ReadStringField(Labels, TEXT("suite")),
          DataAdapters::ReadStringField(Labels, TEXT("npc")),
          DataAdapters::ReadStringField(Labels, TEXT("bridge")),
          DataAdapters::ReadStringField(Labels, TEXT("bridgeList")),
          DataAdapters::ReadStringField(Labels, TEXT("ghost")),
          DataAdapters::ReadStringField(Labels, TEXT("ghostList")),
          DataAdapters::ReadStringField(Labels, TEXT("rule")),
          DataAdapters::ReadStringField(Labels, TEXT("ruleList")),
          DataAdapters::ReadStringField(Labels, TEXT("soul")),
          DataAdapters::ReadStringField(Labels, TEXT("soulList")),
          DataAdapters::ReadStringField(Labels, TEXT("systemTag"))};
}

/** User Story: As an API cache-tag test, I need entity IDs loaded from authored data so tests do not hide identity literals in source. @fn inline FCacheTagIds ReadIds(const DataAdapters::FSettingsSource &Source) */
inline FCacheTagIds ReadIds(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Ids =
      DataAdapters::ReadObjectField(Source, TEXT("ids"));
  return {DataAdapters::ReadStringField(Ids, TEXT("npc")),
          DataAdapters::ReadStringField(Ids, TEXT("bridge")),
          DataAdapters::ReadStringField(Ids, TEXT("ghost")),
          DataAdapters::ReadStringField(Ids, TEXT("rule")),
          DataAdapters::ReadStringField(Ids, TEXT("soul"))};
}

/**
 * User Story: As an API cache-tag test, I need expected cache values loaded as typed RTK Query tags.
 * @fn inline FCacheTagValues ReadTags( const DataAdapters::FSettingsSource &Source)
 */
inline FCacheTagValues ReadTags(
    const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Tags =
      DataAdapters::ReadObjectField(Source, TEXT("tags"));
  return {ReadTag(Tags, TEXT("npc")),
          ReadTag(Tags, TEXT("bridge")),
          ReadTag(Tags, TEXT("bridgeList")),
          ReadTag(Tags, TEXT("ghost")),
          ReadTag(Tags, TEXT("ghostList")),
          ReadTag(Tags, TEXT("rule")),
          ReadTag(Tags, TEXT("ruleList")),
          ReadTag(Tags, TEXT("soul")),
          ReadTag(Tags, TEXT("soulList")),
          ReadTag(Tags, TEXT("system"), false)};
}

/** User Story: As an API cache-tag test, I need one immutable typed fixture shared by all tag assertions. @fn inline const FCacheTagFixtures &cacheTagFixtures() */
inline const FCacheTagFixtures &cacheTagFixtures() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/api/cache-tags.json"));
  static const FCacheTagFixtures Fixtures{
      ReadLabels(Source), ReadIds(Source), ReadTags(Source)};
  return Fixtures;
}

} // namespace Testing::API::CacheTags
