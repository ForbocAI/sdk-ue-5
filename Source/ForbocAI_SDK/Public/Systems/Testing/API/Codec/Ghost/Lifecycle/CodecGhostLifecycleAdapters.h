#pragma once

#include "Components/Testing/API/Codec/Ghost/CodecGhostTypes.h"
#include "Systems/Testing/API/Codec/Ghost/Attribution/AttributionAdapters.h"

namespace Testing::API::Codec::Ghost {

/** User Story: As Ghost lifecycle tests, I need expected run identity read from authored JSON without using the production decoder. @fn inline FGhostRunResponse ReadGhostRunExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostRunResponse
ReadGhostRunExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostRunResponse Expected;
  Expected.SessionId =
      DataAdapters::ReadStringField(Object, TEXT("sessionId"));
  Expected.RunStatus =
      DataAdapters::ReadStringField(Object, TEXT("runStatus"));
  Expected.GhostName =
      DataAdapters::ReadStringField(Object, TEXT("ghostName"));
  Expected.RuntimeIdentity = ReadGhostRuntimeIdentity(
      DataAdapters::ReadObjectField(Object, TEXT("runtimeIdentity")));
  return Expected;
}

/** User Story: As Ghost lifecycle tests, I need expected status evidence read independently from wire decoding. @fn inline FGhostStatus ReadGhostStatusExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostStatus
ReadGhostStatusExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostStatus Expected;
  Expected.SessionId =
      DataAdapters::ReadStringField(Object, TEXT("sessionId"));
  Expected.GhostName =
      DataAdapters::ReadStringField(Object, TEXT("ghostName"));
  Expected.RuntimeIdentity = ReadGhostRuntimeIdentity(
      DataAdapters::ReadObjectField(Object, TEXT("runtimeIdentity")));
  Expected.TestSuite =
      DataAdapters::ReadStringField(Object, TEXT("testSuite"));
  Expected.Status = DataAdapters::ReadStringField(Object, TEXT("status"));
  Expected.Progress =
      DataAdapters::ReadNumberField(Object, TEXT("progress"));
  Expected.StartedAt =
      DataAdapters::ReadStringField(Object, TEXT("startedAt"));
  Expected.Duration =
      DataAdapters::ReadNumberField(Object, TEXT("duration"));
  Expected.Errors = DataAdapters::ReadNumberField(Object, TEXT("errors"));
  return Expected;
}

/** User Story: As Ghost lifecycle tests, I need expected stop state read from authored JSON. @fn inline FGhostStopResponse ReadGhostStopExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostStopResponse
ReadGhostStopExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostStopResponse Expected;
  Expected.bStopped =
      DataAdapters::ReadBooleanField(Object, TEXT("stopped"));
  Expected.StopStatus =
      DataAdapters::ReadStringField(Object, TEXT("stopStatus"));
  Expected.StopSessionId =
      DataAdapters::ReadStringField(Object, TEXT("stopSessionId"));
  return Expected;
}

/** User Story: As Ghost history tests, I need every expected API-attributed history record composed from authored JSON. @fn inline FGhostHistoryEntry ReadGhostHistoryExpectedEntry(const TSharedPtr<FJsonObject> &Object) */
inline FGhostHistoryEntry
ReadGhostHistoryExpectedEntry(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FGhostHistoryEntry Expected;
  Expected.SessionId =
      DataAdapters::ReadStringField(Value, TEXT("sessionId"));
  Expected.GhostName =
      DataAdapters::ReadStringField(Value, TEXT("ghostName"));
  Expected.RuntimeIdentity = ReadGhostRuntimeIdentity(
      DataAdapters::ReadObjectField(Value, TEXT("runtimeIdentity")));
  Expected.TestSuite =
      DataAdapters::ReadStringField(Value, TEXT("testSuite"));
  Expected.StartedAt =
      DataAdapters::ReadStringField(Value, TEXT("startedAt"));
  Expected.CompletedAt = func::match(
      DataAdapters::ReadOptionalStringField(Value, TEXT("completedAt")),
      [](const FString &CompletedAt) { return CompletedAt; },
      []() { return FString(); });
  Expected.Status =
      DataAdapters::ReadStringField(Value, TEXT("status"));
  Expected.PassRate =
      DataAdapters::ReadFloatField(Value, TEXT("passRate"));
  return Expected;
}

/** User Story: As Ghost history tests, I need one typed expected envelope independently composed from authored records. @fn inline FGhostHistoryResponse ReadGhostHistoryExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostHistoryResponse
ReadGhostHistoryExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostHistoryResponse Expected;
  Expected.Sessions = func::map_array<TSharedPtr<FJsonObject>,
                                      FGhostHistoryEntry>(
      DataAdapters::ReadObjectArrayField(Object, TEXT("sessions")),
      ReadGhostHistoryExpectedEntry);
  return Expected;
}

} // namespace Testing::API::Codec::Ghost
