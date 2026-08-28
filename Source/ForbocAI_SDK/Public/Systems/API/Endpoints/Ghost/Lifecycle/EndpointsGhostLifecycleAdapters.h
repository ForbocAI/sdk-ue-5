#pragma once

#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Systems/API/Endpoints/Ghost/Identity/GhostIdentityAdapters.h"

namespace APISlice::Detail {

/** User Story: As Ghost session startup, I need the API-owned name and runtime identity decoded atomically. @fn inline bool DecodeGhostRunResponse(const FString &Json, FGhostRunResponse &Response) */
inline bool DecodeGhostRunResponse(const FString &Json,
                                   FGhostRunResponse &Response) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.Run;
  TSharedPtr<FJsonObject> Root;
  FGhostRunResponse Decoded;
  const bool bRootValid = JsonInterop::ParseJsonObject(Json, Root) &&
                          Root.IsValid();
  const func::Maybe<FGhostRuntimeIdentity> RuntimeIdentity =
      bRootValid ? DecodeGhostRuntimeIdentityField(Root, Fields.RuntimeIdentity)
                 : func::nothing<FGhostRuntimeIdentity>();
  const bool bValid =
      bRootValid &&
      DecodeGhostStringField(Root, Fields.SessionId, Decoded.SessionId) &&
      DecodeGhostStringField(Root, Fields.Status, Decoded.RunStatus) &&
      DecodeGhostIdentityField(Root, Fields.GhostName,
                               Data.Limits.MinimumIdentityLength,
                               Decoded.GhostName) &&
      !func::is_nothing(RuntimeIdentity);
  return !bValid
             ? false
             : (Decoded.RuntimeIdentity = RuntimeIdentity.value,
                Response = Decoded, true);
}

/** User Story: As Ghost status polling, I need all lifecycle and identity fields decoded before state advances. @fn inline bool DecodeGhostStatusResponse(const FString &Json, FGhostStatus &Response) */
inline bool DecodeGhostStatusResponse(const FString &Json,
                                      FGhostStatus &Response) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.Status;
  TSharedPtr<FJsonObject> Root;
  FGhostStatus Decoded;
  const bool bRootValid = JsonInterop::ParseJsonObject(Json, Root) &&
                          Root.IsValid();
  const func::Maybe<FGhostRuntimeIdentity> RuntimeIdentity =
      bRootValid ? DecodeGhostRuntimeIdentityField(Root, Fields.RuntimeIdentity)
                 : func::nothing<FGhostRuntimeIdentity>();
  const bool bValid =
      bRootValid &&
      DecodeGhostStringField(Root, Fields.SessionId, Decoded.SessionId) &&
      DecodeGhostIdentityField(Root, Fields.GhostName,
                               Data.Limits.MinimumIdentityLength,
                               Decoded.GhostName) &&
      !func::is_nothing(RuntimeIdentity) &&
      DecodeGhostStringField(Root, Fields.TestSuite, Decoded.TestSuite) &&
      DecodeGhostStringField(Root, Fields.Status, Decoded.Status) &&
      DecodeGhostInt32Field(Root, Fields.Progress, Decoded.Progress) &&
      DecodeGhostStringField(Root, Fields.StartedAt, Decoded.StartedAt) &&
      DecodeGhostInt32Field(Root, Fields.Duration, Decoded.Duration) &&
      DecodeGhostInt32Field(Root, Fields.Errors, Decoded.Errors);
  return !bValid
             ? false
             : (Decoded.RuntimeIdentity = RuntimeIdentity.value,
                Response = Decoded, true);
}

/** User Story: As Ghost lifecycle control, I need stop metadata decoded without inventing completion state. @fn inline bool DecodeGhostStopResponse(const FString &Json, FGhostStopResponse &Response) */
inline bool DecodeGhostStopResponse(const FString &Json,
                                    FGhostStopResponse &Response) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.Stop;
  TSharedPtr<FJsonObject> Root;
  FGhostStopResponse Decoded;
  const bool bValid = JsonInterop::ParseJsonObject(Json, Root) &&
                      Root.IsValid() &&
                      DecodeGhostStringField(Root, Fields.Status,
                                             Decoded.StopStatus) &&
                      DecodeGhostStringField(Root, Fields.SessionId,
                                             Decoded.StopSessionId);
  return !bValid
             ? false
             : (Decoded.bStopped = Decoded.StopStatus.Equals(
                    Data.Values.Stopped, ESearchCase::CaseSensitive),
                Response = Decoded, true);
}

/**
 * User Story: As Ghost history decoding, I need every session record preserve API identity and optional completion metadata.
 * @fn inline func::Maybe<FGhostHistoryEntry> DecodeGhostHistoryEntryObject( const TSharedPtr<FJsonObject> &Object)
 */
inline func::Maybe<FGhostHistoryEntry> DecodeGhostHistoryEntryObject(
    const TSharedPtr<FJsonObject> &Object) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.History;
  FGhostHistoryEntry Decoded;
  const func::Maybe<FGhostRuntimeIdentity> RuntimeIdentity =
      DecodeGhostRuntimeIdentityField(Object, Fields.RuntimeIdentity);
  const bool bValid =
      DecodeGhostStringField(Object, Fields.SessionId, Decoded.SessionId) &&
      DecodeGhostIdentityField(Object, Fields.GhostName,
                               Data.Limits.MinimumIdentityLength,
                               Decoded.GhostName) &&
      !func::is_nothing(RuntimeIdentity) &&
      DecodeGhostStringField(Object, Fields.TestSuite, Decoded.TestSuite) &&
      DecodeGhostStringField(Object, Fields.StartedAt, Decoded.StartedAt) &&
      DecodeGhostOptionalStringField(Object, Fields.CompletedAt,
                                     Decoded.CompletedAt) &&
      DecodeGhostStringField(Object, Fields.Status, Decoded.Status) &&
      DecodeGhostFloatField(Object, Fields.PassRate, Decoded.PassRate);
  return !bValid
             ? func::nothing<FGhostHistoryEntry>()
             : (Decoded.RuntimeIdentity = RuntimeIdentity.value,
                func::just(Decoded));
}

/**
 * User Story: As strict Ghost history traversal, I need non-object entries reject the complete response.
 * @fn inline func::Maybe<FGhostHistoryEntry> DecodeGhostHistoryEntryValue( const TSharedPtr<FJsonValue> &Value)
 */
inline func::Maybe<FGhostHistoryEntry> DecodeGhostHistoryEntryValue(
    const TSharedPtr<FJsonValue> &Value) {
  return Value.IsValid() && Value->Type == EJson::Object
             ? DecodeGhostHistoryEntryObject(Value->AsObject())
             : func::nothing<FGhostHistoryEntry>();
}

/** User Story: As Ghost history consumers, I need malformed nested sessions reject the complete history envelope. @fn inline bool DecodeGhostHistoryResponse(const FString &Json, FGhostHistoryResponse &Response) */
inline bool DecodeGhostHistoryResponse(const FString &Json,
                                       FGhostHistoryResponse &Response) {
  const auto &Fields =
      Endpoints::GhostConfiguration::ghostApiConfiguration().Fields.History;
  TSharedPtr<FJsonObject> Root;
  const TArray<TSharedPtr<FJsonValue>> *Sessions = nullptr;
  const bool bValidRoot = JsonInterop::ParseJsonObject(Json, Root) &&
                          Root.IsValid() &&
                          Root->TryGetArrayField(Fields.Sessions, Sessions) &&
                          Sessions;
  const func::Maybe<TArray<FGhostHistoryEntry>> Decoded =
      bValidRoot
          ? func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                       FGhostHistoryEntry>(
                *Sessions, DecodeGhostHistoryEntryValue)
          : func::nothing<TArray<FGhostHistoryEntry>>();
  return func::is_nothing(Decoded)
             ? false
             : (Response.Sessions = Decoded.value, true);
}

} // namespace APISlice::Detail
