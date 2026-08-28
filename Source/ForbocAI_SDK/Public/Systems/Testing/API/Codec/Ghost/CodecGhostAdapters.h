#pragma once

#include "Systems/Testing/API/Codec/Ghost/Lifecycle/CodecGhostLifecycleAdapters.h"
#include "Systems/Testing/API/Codec/Ghost/Results/CodecGhostResultsAdapters.h"

namespace Testing::API::Codec::Ghost {

/** User Story: As Ghost codec tests, I need assertion labels loaded from the same authored fixture catalog as response data. @fn inline FGhostCodecLabels ReadGhostCodecLabels(const TSharedRef<FJsonObject> &Object) */
inline FGhostCodecLabels
ReadGhostCodecLabels(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("run")),
      DataAdapters::ReadStringField(Object, TEXT("status")),
      DataAdapters::ReadStringField(Object, TEXT("results")),
      DataAdapters::ReadStringField(Object, TEXT("stop")),
      DataAdapters::ReadStringField(Object, TEXT("history")),
      DataAdapters::ReadStringField(Object, TEXT("sessionId")),
      DataAdapters::ReadStringField(Object, TEXT("ghostName")),
      DataAdapters::ReadStringField(Object, TEXT("runtimeIdentity")),
      DataAdapters::ReadStringField(Object, TEXT("testSuite")),
      DataAdapters::ReadStringField(Object, TEXT("progress")),
      DataAdapters::ReadStringField(Object, TEXT("evidence")),
      DataAdapters::ReadStringField(Object, TEXT("evidenceDimensions")),
      DataAdapters::ReadStringField(Object, TEXT("evaluationDimensions")),
      DataAdapters::ReadStringField(Object, TEXT("testCount")),
      DataAdapters::ReadStringField(Object, TEXT("coverage")),
      DataAdapters::ReadStringField(Object, TEXT("metric")),
      DataAdapters::ReadStringField(Object, TEXT("verdict")),
      DataAdapters::ReadStringField(Object, TEXT("summary")),
      DataAdapters::ReadStringField(Object, TEXT("rejectsMissingIdentity")),
      DataAdapters::ReadStringField(Object, TEXT("rejectsEmptyIdentity")),
      DataAdapters::ReadStringField(Object, TEXT("rejectsMalformedTest")),
      DataAdapters::ReadStringField(Object, TEXT("rejectsMalformedMetric")),
      DataAdapters::ReadStringField(Object, TEXT("rejectsMalformedHistory")),
  };
}

/** User Story: As strict Ghost codec tests, I need malformed nested-value instructions authored outside test code. @fn inline FGhostCodecMalformedFixture ReadGhostCodecMalformedFixture(const TSharedRef<FJsonObject> &Object) */
inline FGhostCodecMalformedFixture
ReadGhostCodecMalformedFixture(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
          Object, TEXT("runMissingIdentity"))),
      DataAdapters::SerializeObject(DataAdapters::ReadObjectField(
          Object, TEXT("runEmptyIdentity"))),
      DataAdapters::ReadNumberField(Object, TEXT("testIndex")),
      DataAdapters::ReadStringField(Object, TEXT("testReplacement")),
      DataAdapters::ReadNumberField(Object, TEXT("metricIndex")),
      DataAdapters::ReadStringField(Object, TEXT("metricReplacement")),
      DataAdapters::ReadNumberField(Object, TEXT("historyIndex")),
      DataAdapters::ReadStringField(Object, TEXT("historyReplacement")),
  };
}

/** User Story: As Ghost codec tests, I need probe indices and metric expectations authored with the contract examples. @fn inline FGhostCodecProbeFixture ReadGhostCodecProbeFixture(const TSharedRef<FJsonObject> &Object) */
inline FGhostCodecProbeFixture
ReadGhostCodecProbeFixture(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadNumberField(Object, TEXT("evidenceDimensionIndex")),
      DataAdapters::ReadNumberField(Object,
                                    TEXT("evaluationDimensionIndex")),
      DataAdapters::ReadNumberField(Object, TEXT("testIndex")),
      DataAdapters::ReadNumberField(Object, TEXT("historyIndex")),
      DataAdapters::ReadStringField(Object, TEXT("metricName")),
      DataAdapters::ReadFloatField(Object, TEXT("metricValue")),
  };
}

/** User Story: As UE and TS parity verification, I need one immutable Ghost fixture composed from the shared wire examples. @fn inline const FGhostCodecFixture &GhostCodecFixtures() */
inline const FGhostCodecFixture &GhostCodecFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/api/ghost.json"));
  static const FGhostCodecFixture Fixture = {
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Source, TEXT("runResponse"))),
      ReadGhostRunExpected(
          DataAdapters::ReadObjectField(Source, TEXT("runExpected"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Source, TEXT("statusResponse"))),
      ReadGhostStatusExpected(
          DataAdapters::ReadObjectField(Source, TEXT("statusExpected"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Source, TEXT("resultsResponse"))),
      ReadGhostResultsExpected(
          DataAdapters::ReadObjectField(Source, TEXT("resultsExpected"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Source, TEXT("stopResponse"))),
      ReadGhostStopExpected(
          DataAdapters::ReadObjectField(Source, TEXT("stopExpected"))),
      DataAdapters::SerializeObject(
          DataAdapters::ReadObjectField(Source, TEXT("historyResponse"))),
      ReadGhostHistoryExpected(
          DataAdapters::ReadObjectField(Source, TEXT("historyExpected"))),
      ReadGhostCodecMalformedFixture(
          DataAdapters::ReadObjectField(Source, TEXT("malformed"))),
      ReadGhostCodecProbeFixture(
          DataAdapters::ReadObjectField(Source, TEXT("probes"))),
      ReadGhostCodecLabels(
          DataAdapters::ReadObjectField(Source, TEXT("labels"))),
  };
  return Fixture;
}

} // namespace Testing::API::Codec::Ghost
