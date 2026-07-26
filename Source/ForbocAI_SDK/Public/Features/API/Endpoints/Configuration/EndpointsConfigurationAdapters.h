#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/API/Endpoints/Configuration/EndpointsConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "GenericPlatform/GenericPlatformHttp.h"

namespace APISlice::Endpoints::Configuration {

/**
 * @fn inline FEndpointConfigurationData readEndpointConfigurationData()
 * User Story: As an API contract maintainer, I need endpoint policy loaded from the shared authored contract so TS and UE use the same routes, names, wire fields, and defaults.
 */
inline FEndpointConfigurationData readEndpointConfigurationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/endpoints.json"));
  const TSharedRef<FJsonObject> Path =
      DataAdapters::ReadObjectField(Source, TEXT("path"));
  const TSharedRef<FJsonObject> Segments =
      DataAdapters::ReadObjectField(Source, TEXT("segments"));
  const TSharedRef<FJsonObject> QueryKeys =
      DataAdapters::ReadObjectField(Source, TEXT("queryKeys"));
  const TSharedRef<FJsonObject> Names =
      DataAdapters::ReadObjectField(Source, TEXT("names"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Structures =
      DataAdapters::ReadObjectField(Source, TEXT("structures"));
  const TSharedRef<FJsonObject> GhostMetric =
      DataAdapters::ReadObjectField(Structures, TEXT("ghostMetric"));
  const TSharedRef<FJsonObject> Values =
      DataAdapters::ReadObjectField(Source, TEXT("values"));
  const TSharedRef<FJsonObject> Payloads =
      DataAdapters::ReadObjectField(Source, TEXT("payloads"));
  const TSharedRef<FJsonObject> TagIds =
      DataAdapters::ReadObjectField(Source, TEXT("tagIds"));
  const TSharedRef<FJsonObject> Defaults =
      DataAdapters::ReadObjectField(Source, TEXT("defaults"));
  const TSharedRef<FJsonObject> Timeouts =
      DataAdapters::ReadObjectField(Source, TEXT("timeouts"));

  return {
      {DataAdapters::ReadStringField(Path, TEXT("separator")),
       DataAdapters::ReadStringField(Path, TEXT("queryStart")),
       DataAdapters::ReadStringField(Path, TEXT("queryAssignment"))},
      {DataAdapters::ReadStringField(Segments, TEXT("bridge")),
       DataAdapters::ReadStringField(Segments, TEXT("validate")),
       DataAdapters::ReadStringField(Segments, TEXT("rules")),
       DataAdapters::ReadStringField(Segments, TEXT("ghost")),
       DataAdapters::ReadStringField(Segments, TEXT("run")),
       DataAdapters::ReadStringField(Segments, TEXT("status")),
       DataAdapters::ReadStringField(Segments, TEXT("results")),
       DataAdapters::ReadStringField(Segments, TEXT("stop")),
       DataAdapters::ReadStringField(Segments, TEXT("history")),
       DataAdapters::ReadStringField(Segments, TEXT("npcs")),
       DataAdapters::ReadStringField(Segments, TEXT("process")),
       DataAdapters::ReadStringField(Segments, TEXT("conversation")),
       DataAdapters::ReadStringField(Segments, TEXT("presets")),
       DataAdapters::ReadStringField(Segments, TEXT("soul")),
       DataAdapters::ReadStringField(Segments, TEXT("export")),
       DataAdapters::ReadStringField(Segments, TEXT("confirm")),
       DataAdapters::ReadStringField(Segments, TEXT("souls")),
       DataAdapters::ReadStringField(Segments, TEXT("verify")),
       DataAdapters::ReadStringField(Segments, TEXT("testGame")),
       DataAdapters::ReadStringField(Segments, TEXT("contract"))},
      {DataAdapters::ReadStringField(QueryKeys, TEXT("limit"))},
      {DataAdapters::ReadStringField(Names, TEXT("getApiStatus")),
       DataAdapters::ReadStringField(Names, TEXT("postNpcProcess")),
       DataAdapters::ReadStringField(Names, TEXT("postNpcConversation")),
       DataAdapters::ReadStringField(Names, TEXT("getBridgeValidation")),
       DataAdapters::ReadStringField(Names, TEXT("getBridgeRules")),
       DataAdapters::ReadStringField(Names, TEXT("postGhostRun")),
       DataAdapters::ReadStringField(Names, TEXT("getGhostStatus")),
       DataAdapters::ReadStringField(Names, TEXT("getGhostResults")),
       DataAdapters::ReadStringField(Names, TEXT("postGhostStop")),
       DataAdapters::ReadStringField(Names, TEXT("getGhostHistory")),
       DataAdapters::ReadStringField(Names, TEXT("postBridgePreset")),
       DataAdapters::ReadStringField(Names, TEXT("getRulesets")),
       DataAdapters::ReadStringField(Names, TEXT("getRulePresets")),
       DataAdapters::ReadStringField(Names,
                                     TEXT("postSoulExportPreparation")),
       DataAdapters::ReadStringField(Names,
                                     TEXT("postSoulExportConfirmation")),
       DataAdapters::ReadStringField(Names, TEXT("postSoulVerification")),
       DataAdapters::ReadStringField(Names, TEXT("postSoulStorageUpload")),
       DataAdapters::ReadStringField(Names, TEXT("getSoulStorageDownload")),
       DataAdapters::ReadStringField(Names,
                                     TEXT("getSoulStorageVerification")),
       DataAdapters::ReadStringField(Names,
                                     TEXT("postSoulStoragePreparation")),
       DataAdapters::ReadStringField(Names,
                                     TEXT("deleteSoulStoragePreparation")),
       DataAdapters::ReadStringField(Names, TEXT("postSoulStorageCommit")),
       DataAdapters::ReadStringField(Names, TEXT("getSoulStorageCatalog")),
       DataAdapters::ReadStringField(Names, TEXT("getSoulStorageEntry")),
       DataAdapters::ReadStringField(Names, TEXT("getTestGameContract"))},
      {DataAdapters::ReadStringField(Fields, TEXT("ghostRunSessionId")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostRunStatus")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusSessionId")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusStatus")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusProgress")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusStartedAt")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusDuration")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStatusErrors")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsSessionId")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsTotalTests")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsPassed")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsFailed")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsSkipped")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsDuration")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsTests")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsCoverage")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostResultsMetrics")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostTestName")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostTestPassed")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostTestDuration")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostTestError")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostTestScreenshot")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStopStatus")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostStopSessionId")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistorySessions")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistorySessionId")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistoryTestSuite")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistoryStartedAt")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistoryCompletedAt")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistoryStatus")),
       DataAdapters::ReadStringField(Fields, TEXT("ghostHistoryPassRate"))},
      {{DataAdapters::ReadNumberField(GhostMetric, TEXT("pairSize")),
        DataAdapters::ReadNumberField(GhostMetric, TEXT("keyIndex")),
        DataAdapters::ReadNumberField(GhostMetric, TEXT("valueIndex"))}},
      {DataAdapters::ReadStringField(Values, TEXT("stopped"))},
      {DataAdapters::ReadStringField(Payloads, TEXT("emptyObject"))},
      {DataAdapters::ReadStringField(TagIds, TEXT("list"))},
      {DataAdapters::ReadNumberField(Defaults, TEXT("ghostHistoryLimit")),
       DataAdapters::ReadNumberField(Defaults, TEXT("soulListLimit"))},
      {DataAdapters::ReadNumberField(Timeouts, TEXT("statusMs")),
       DataAdapters::ReadNumberField(Timeouts, TEXT("contractMs")),
       DataAdapters::ReadNumberField(Timeouts, TEXT("npcProcessMs")),
       DataAdapters::ReadNumberField(Timeouts, TEXT("npcConversationMs"))}};
}

/**
 * @fn inline const FEndpointConfigurationData &endpointData()
 * User Story: As an API endpoint consumer, I need one immutable endpoint contract per process so every endpoint composes the same authored values.
 */
inline const FEndpointConfigurationData &endpointData() {
  static const FEndpointConfigurationData Data = readEndpointConfigurationData();
  return Data;
}

/**
 * @fn inline TArray<FString> endpointNames(const FEndpointNameData &Names)
 * User Story: As an API inventory consumer, I need every authored endpoint
 * name projected from typed configuration so registry completeness is dynamic.
 */
inline TArray<FString> endpointNames(const FEndpointNameData &Names) {
  return {Names.GetApiStatus,
          Names.PostNpcProcess,
          Names.PostNpcConversation,
          Names.GetBridgeValidation,
          Names.GetBridgeRules,
          Names.PostGhostRun,
          Names.GetGhostStatus,
          Names.GetGhostResults,
          Names.PostGhostStop,
          Names.GetGhostHistory,
          Names.PostBridgePreset,
          Names.GetRulesets,
          Names.GetRulePresets,
          Names.PostSoulExportPreparation,
          Names.PostSoulExportConfirmation,
          Names.PostSoulVerification,
          Names.PostSoulStorageUpload,
          Names.GetSoulStorageDownload,
          Names.GetSoulStorageVerification,
          Names.PostSoulStoragePreparation,
          Names.DeleteSoulStoragePreparation,
          Names.PostSoulStorageCommit,
          Names.GetSoulStorageCatalog,
          Names.GetSoulStorageEntry,
          Names.GetTestGameContract};
}

/**
 * @fn inline FString encodePathSegment(const FString &Value)
 * User Story: As an API endpoint consumer, I need dynamic identifiers encoded before route composition so values cannot alter route structure.
 */
inline FString encodePathSegment(const FString &Value) {
  return FGenericPlatformHttp::UrlEncode(Value);
}

/**
 * @fn inline FString endpointPath(const TArray<FString> &Segments)
 * User Story: As an API endpoint maintainer, I need FP route composition so every static and dynamic segment follows one encoding policy.
 */
inline FString endpointPath(const TArray<FString> &Segments) {
  const FEndpointConfigurationData &Data = endpointData();
  const TArray<FString> Encoded = func::map_array<FString, FString>(
      Segments, [](const FString &Segment) { return encodePathSegment(Segment); });
  return Data.Path.Separator + FString::Join(Encoded, *Data.Path.Separator);
}

/**
 * @fn inline FString endpointQuery(const FString &Path, const FString &Key, const FString &Value)
 * User Story: As an API endpoint maintainer, I need query components encoded through one function so pagination remains portable and deterministic.
 */
inline FString endpointQuery(const FString &Path, const FString &Key,
                             const FString &Value) {
  const FEndpointConfigurationData &Data = endpointData();
  return Path + Data.Path.QueryStart + FGenericPlatformHttp::UrlEncode(Key) +
         Data.Path.QueryAssignment + FGenericPlatformHttp::UrlEncode(Value);
}

/**
 * @fn inline FString apiEndpoint(const FString &ApiUrl, const FString &Path)
 * User Story: As an API endpoint consumer, I need relative authored routes resolved against the configured API origin in one place.
 */
inline FString apiEndpoint(const FString &ApiUrl, const FString &Path) {
  return ApiUrl + Path;
}

/**
 * @fn inline FApiEndpointTag endpointTag(const FString &Type, const FString &Id = FString())
 * User Story: As an RTK Query endpoint maintainer, I need tag construction centralized so cache ownership remains explicit and composable.
 */
inline FApiEndpointTag endpointTag(const FString &Type,
                                   const FString &Id = FString()) {
  return FApiEndpointTag{Type, Id};
}

/**
 * @fn inline FApiEndpointTag endpointListTag(const FString &Type)
 * User Story: As an RTK Query catalog owner, I need one authored list identity so collection reads and writes invalidate the same cache entry.
 */
inline FApiEndpointTag endpointListTag(const FString &Type) {
  return endpointTag(Type, endpointData().TagIds.List);
}

} // namespace APISlice::Endpoints::Configuration
