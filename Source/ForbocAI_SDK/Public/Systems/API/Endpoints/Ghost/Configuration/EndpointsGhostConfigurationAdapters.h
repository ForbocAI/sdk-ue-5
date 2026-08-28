#pragma once

#include "Components/API/Endpoints/Ghost/Configuration/EndpointsGhostConfigurationTypes.h"
#include "Systems/Data/DataAdapters.h"

namespace APISlice::Endpoints::GhostConfiguration {

/** User Story: As strict Ghost codecs, I need the complete wire contract loaded from one API-owned JSON mirror. @fn inline FGhostApiConfigurationData readGhostApiConfiguration() */
inline FGhostApiConfigurationData readGhostApiConfiguration() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/api/ghost.json"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Run =
      DataAdapters::ReadObjectField(Fields, TEXT("run"));
  const TSharedRef<FJsonObject> RuntimeIdentity =
      DataAdapters::ReadObjectField(Fields, TEXT("runtimeIdentity"));
  const TSharedRef<FJsonObject> Status =
      DataAdapters::ReadObjectField(Fields, TEXT("status"));
  const TSharedRef<FJsonObject> Results =
      DataAdapters::ReadObjectField(Fields, TEXT("results"));
  const TSharedRef<FJsonObject> Evidence =
      DataAdapters::ReadObjectField(Fields, TEXT("evidence"));
  const TSharedRef<FJsonObject> Test =
      DataAdapters::ReadObjectField(Fields, TEXT("test"));
  const TSharedRef<FJsonObject> Stop =
      DataAdapters::ReadObjectField(Fields, TEXT("stop"));
  const TSharedRef<FJsonObject> History =
      DataAdapters::ReadObjectField(Fields, TEXT("history"));
  const TSharedRef<FJsonObject> Metric =
      DataAdapters::ReadObjectField(Fields, TEXT("metric"));
  const TSharedRef<FJsonObject> Values =
      DataAdapters::ReadObjectField(Source, TEXT("values"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Errors =
      DataAdapters::ReadObjectField(Source, TEXT("errors"));

  return {
      {{DataAdapters::ReadStringField(Run, TEXT("sessionId")),
        DataAdapters::ReadStringField(Run, TEXT("status")),
        DataAdapters::ReadStringField(Run, TEXT("ghostName")),
        DataAdapters::ReadStringField(Run, TEXT("runtimeIdentity"))},
       {DataAdapters::ReadStringField(RuntimeIdentity, TEXT("apiVersion")),
        DataAdapters::ReadStringField(RuntimeIdentity, TEXT("slmStatus")),
        DataAdapters::ReadStringField(RuntimeIdentity, TEXT("slmVersion")),
        DataAdapters::ReadStringField(RuntimeIdentity,
                                      TEXT("slotContractVersion"))},
       {DataAdapters::ReadStringField(Status, TEXT("sessionId")),
        DataAdapters::ReadStringField(Status, TEXT("ghostName")),
        DataAdapters::ReadStringField(Status, TEXT("runtimeIdentity")),
        DataAdapters::ReadStringField(Status, TEXT("testSuite")),
        DataAdapters::ReadStringField(Status, TEXT("status")),
        DataAdapters::ReadStringField(Status, TEXT("progress")),
        DataAdapters::ReadStringField(Status, TEXT("startedAt")),
        DataAdapters::ReadStringField(Status, TEXT("duration")),
        DataAdapters::ReadStringField(Status, TEXT("errors"))},
       {DataAdapters::ReadStringField(Results, TEXT("sessionId")),
        DataAdapters::ReadStringField(Results, TEXT("ghostName")),
        DataAdapters::ReadStringField(Results, TEXT("runtimeIdentity")),
        DataAdapters::ReadStringField(Results, TEXT("evidence")),
        DataAdapters::ReadStringField(Results, TEXT("evidenceDimensions")),
        DataAdapters::ReadStringField(Results, TEXT("evaluationDimensions")),
        DataAdapters::ReadStringField(Results, TEXT("totalTests")),
        DataAdapters::ReadStringField(Results, TEXT("passed")),
        DataAdapters::ReadStringField(Results, TEXT("failed")),
        DataAdapters::ReadStringField(Results, TEXT("skipped")),
        DataAdapters::ReadStringField(Results, TEXT("duration")),
        DataAdapters::ReadStringField(Results, TEXT("tests")),
        DataAdapters::ReadStringField(Results, TEXT("coverage")),
        DataAdapters::ReadStringField(Results, TEXT("metrics")),
        DataAdapters::ReadStringField(Results, TEXT("verdict")),
        DataAdapters::ReadStringField(Results, TEXT("summary"))},
       {DataAdapters::ReadStringField(Evidence, TEXT("completedProcessTurns")),
        DataAdapters::ReadStringField(Evidence, TEXT("validCognitionOutcomes")),
        DataAdapters::ReadStringField(Evidence, TEXT("invalidCognitionOutcomes")),
        DataAdapters::ReadStringField(Evidence, TEXT("signedCognitionOutcomes")),
        DataAdapters::ReadStringField(Evidence, TEXT("thoughtResults")),
        DataAdapters::ReadStringField(Evidence, TEXT("reasoningResults")),
        DataAdapters::ReadStringField(Evidence, TEXT("consistentReasoningResults")),
        DataAdapters::ReadStringField(Evidence, TEXT("diagnosticResults")),
        DataAdapters::ReadStringField(Evidence, TEXT("groundedDiagnosticResults")),
        DataAdapters::ReadStringField(Evidence, TEXT("contradictions")),
        DataAdapters::ReadStringField(Evidence, TEXT("unsupportedClaims")),
        DataAdapters::ReadStringField(Evidence, TEXT("failures")),
        DataAdapters::ReadStringField(Evidence, TEXT("timeouts")),
        DataAdapters::ReadStringField(Evidence, TEXT("uniqueObservations")),
        DataAdapters::ReadStringField(Evidence, TEXT("choicePoints"))},
       {DataAdapters::ReadStringField(Test, TEXT("name")),
        DataAdapters::ReadStringField(Test, TEXT("passed")),
        DataAdapters::ReadStringField(Test, TEXT("duration")),
        DataAdapters::ReadStringField(Test, TEXT("error")),
        DataAdapters::ReadStringField(Test, TEXT("screenshot"))},
       {DataAdapters::ReadStringField(Stop, TEXT("status")),
        DataAdapters::ReadStringField(Stop, TEXT("sessionId"))},
       {DataAdapters::ReadStringField(History, TEXT("sessions")),
        DataAdapters::ReadStringField(History, TEXT("sessionId")),
        DataAdapters::ReadStringField(History, TEXT("ghostName")),
        DataAdapters::ReadStringField(History, TEXT("runtimeIdentity")),
        DataAdapters::ReadStringField(History, TEXT("testSuite")),
        DataAdapters::ReadStringField(History, TEXT("startedAt")),
        DataAdapters::ReadStringField(History, TEXT("completedAt")),
        DataAdapters::ReadStringField(History, TEXT("status")),
        DataAdapters::ReadStringField(History, TEXT("passRate"))},
       {DataAdapters::ReadStringField(Metric, TEXT("name")),
        DataAdapters::ReadStringField(Metric, TEXT("value"))}},
      {DataAdapters::ReadStringField(Values, TEXT("stopped"))},
      {DataAdapters::ReadNumberField(Limits, TEXT("minimumIdentityLength")),
       DataAdapters::ReadNumberField(Limits, TEXT("metricPairSize")),
       DataAdapters::ReadNumberField(Limits, TEXT("metricNameIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("metricValueIndex"))},
      {DataAdapters::ReadStringField(Errors, TEXT("run")),
       DataAdapters::ReadStringField(Errors, TEXT("identity")),
       DataAdapters::ReadStringField(Errors, TEXT("runtimeIdentity")),
       DataAdapters::ReadStringField(Errors, TEXT("status")),
       DataAdapters::ReadStringField(Errors, TEXT("results")),
       DataAdapters::ReadStringField(Errors, TEXT("evidence")),
       DataAdapters::ReadStringField(Errors, TEXT("test")),
       DataAdapters::ReadStringField(Errors, TEXT("stop")),
       DataAdapters::ReadStringField(Errors, TEXT("history")),
       DataAdapters::ReadStringField(Errors, TEXT("metric"))}};
}

/** User Story: As Ghost endpoint codecs, I need one immutable wire contract per process. @fn inline const FGhostApiConfigurationData &ghostApiConfiguration() */
inline const FGhostApiConfigurationData &ghostApiConfiguration() {
  static const FGhostApiConfigurationData Data = readGhostApiConfiguration();
  return Data;
}

} // namespace APISlice::Endpoints::GhostConfiguration
