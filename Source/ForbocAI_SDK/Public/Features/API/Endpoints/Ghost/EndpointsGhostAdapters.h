#pragma once

#include "Core/JsonInterop.h"
#include "Features/API/Endpoints/Configuration/EndpointsConfigurationAdapters.h"
#include "Features/API/Transport/Configuration/TransportConfigurationAdapters.h"
#include "Features/Ghost/GhostTypes.h"

namespace APISlice::Endpoints {

/** User Story: As the Ghost cache owner, I need each run identified by session so polling and stop operations share one entity cache. @fn inline FApiEndpointTag ghostTagAdapter(const FString &SessionId) */
inline FApiEndpointTag ghostTagAdapter(const FString &SessionId) {
  return Configuration::endpointTag(
      Transport::transportQueryData().Tags.Ghost, SessionId);
}

/** User Story: As the Ghost cache owner, I need one authored list identity for run history. @fn inline FApiEndpointTag ghostListTagAdapter() */
inline FApiEndpointTag ghostListTagAdapter() {
  return Configuration::endpointListTag(
      Transport::transportQueryData().Tags.Ghost);
}

} // namespace APISlice::Endpoints

namespace APISlice::Detail {

/**
 * @fn inline bool DecodeGhostRunResponse(const FString &Json, FGhostRunResponse &Response)
 * User Story: As a Ghost consumer, I need run metadata normalized at the RTK Query boundary so feature state receives domain names only.
 */
inline bool DecodeGhostRunResponse(const FString &Json,
                                   FGhostRunResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.SessionId = JsonInterop::OptionalStringFromField(
                    Root, Fields.GhostRunSessionId),
                Response.RunStatus = JsonInterop::OptionalStringFromField(
                    Root, Fields.GhostRunStatus),
                true);
}

/**
 * @fn inline FGhostResultRecord DecodeGhostTestRecord(const TSharedPtr<FJsonObject> &Object)
 * User Story: As a Ghost consumer, I need each transport test record transformed into one stable domain record through the authored wire contract.
 */
inline FGhostResultRecord
DecodeGhostTestRecord(const TSharedPtr<FJsonObject> &Object) {
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  FGhostResultRecord Record;
  return (Record.TestName = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostTestName),
          Record.bTestPassed = JsonInterop::detail::TryGetBoolAs(
              Object, Fields.GhostTestPassed, Record.bTestPassed),
          Record.TestDuration = JsonInterop::detail::TryGetNumberAs<int64>(
              Object, Fields.GhostTestDuration, Record.TestDuration),
          Record.TestError = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostTestError),
          Record.TestScreenshot = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostTestScreenshot),
          Record);
}

/**
 * @fn inline TArray<FGhostResultRecord> DecodeGhostTestRecords(const TArray<TSharedPtr<FJsonValue>> &Values)
 * User Story: As a Ghost consumer, I need test arrays transformed with FP collection operators so malformed non-object values cannot enter domain state.
 */
inline TArray<FGhostResultRecord>
DecodeGhostTestRecords(const TArray<TSharedPtr<FJsonValue>> &Values) {
  return func::filter_map_array<TSharedPtr<FJsonValue>, FGhostResultRecord>(
      Values,
      [](const TSharedPtr<FJsonValue> &Value) {
        return Value.IsValid() && Value->Type == EJson::Object;
      },
      [](const TSharedPtr<FJsonValue> &Value) {
        return DecodeGhostTestRecord(Value->AsObject());
      });
}

/**
 * @fn inline func::Maybe<TPair<FString, float>> DecodeGhostMetricPair(const TSharedPtr<FJsonValue> &Value)
 * User Story: As a Ghost consumer, I need metric tuple validation isolated at the wire boundary so malformed tuples are omitted rather than fabricated.
 */
inline func::Maybe<TPair<FString, float>>
DecodeGhostMetricPair(const TSharedPtr<FJsonValue> &Value) {
  const Endpoints::Configuration::FEndpointTupleStructureData &Structure =
      Endpoints::Configuration::endpointData().Structures.GhostMetric;
  return !Value.IsValid() || Value->Type != EJson::Array
             ? func::nothing<TPair<FString, float>>()
             : [&]() {
                 const TArray<TSharedPtr<FJsonValue>> &Pair = Value->AsArray();
                 const bool bValid =
                     Pair.Num() == Structure.PairSize &&
                     Pair.IsValidIndex(Structure.KeyIndex) &&
                     Pair.IsValidIndex(Structure.ValueIndex) &&
                     Pair[Structure.KeyIndex].IsValid() &&
                     Pair[Structure.KeyIndex]->Type == EJson::String &&
                     Pair[Structure.ValueIndex].IsValid() &&
                     Pair[Structure.ValueIndex]->Type == EJson::Number &&
                     FMath::IsFinite(Pair[Structure.ValueIndex]->AsNumber());
                 return !bValid
                            ? func::nothing<TPair<FString, float>>()
                            : func::just<TPair<FString, float>>(
                                  TPair<FString, float>(
                                      Pair[Structure.KeyIndex]->AsString(),
                                      static_cast<float>(
                                          Pair[Structure.ValueIndex]
                                              ->AsNumber())));
               }();
}

/**
 * @fn inline TMap<FString, float> DecodeGhostMetrics(const TArray<TSharedPtr<FJsonValue>> &Values)
 * User Story: As a Ghost consumer, I need valid metric tuples folded into domain state without index-driven parsing or mutation outside the adapter.
 */
inline TMap<FString, float>
DecodeGhostMetrics(const TArray<TSharedPtr<FJsonValue>> &Values) {
  return func::fold_array<TSharedPtr<FJsonValue>, TMap<FString, float>>(
      Values, TMap<FString, float>(),
      [](const TMap<FString, float> &Metrics,
         const TSharedPtr<FJsonValue> &Value) {
        return func::match(
            DecodeGhostMetricPair(Value),
            [&Metrics](const TPair<FString, float> &Metric) {
              TMap<FString, float> Updated = Metrics;
              Updated.Add(Metric.Key, Metric.Value);
              return Updated;
            },
            [&Metrics]() { return Metrics; });
      });
}

/**
 * @fn inline bool DecodeGhostStatusResponse(const FString &Json, FGhostStatus &Response)
 * User Story: As a Ghost consumer, I need status wire fields transformed once so thunks and selectors consume the same domain contract as TS.
 */
inline bool DecodeGhostStatusResponse(const FString &Json,
                                      FGhostStatus &Response) {
  TSharedPtr<FJsonObject> Root;
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.SessionId = JsonInterop::OptionalStringFromField(
                    Root, Fields.GhostStatusSessionId),
                Response.Status = JsonInterop::OptionalStringFromField(
                    Root, Fields.GhostStatusStatus),
                Response.Progress = JsonInterop::detail::TryGetNumberAs<float>(
                    Root, Fields.GhostStatusProgress, Response.Progress),
                Response.StartedAt = JsonInterop::OptionalStringFromField(
                    Root, Fields.GhostStatusStartedAt),
                Response.Duration = JsonInterop::detail::TryGetNumberAs<int32>(
                    Root, Fields.GhostStatusDuration, Response.Duration),
                Response.Errors = JsonInterop::detail::TryGetNumberAs<int32>(
                    Root, Fields.GhostStatusErrors, Response.Errors),
                true);
}

/**
 * @fn inline bool DecodeGhostResultsResponse(const FString &Json, FGhostResults &Response)
 * User Story: As a Ghost consumer, I need aggregate and per-test wire data normalized at the RTK Query boundary so feature thunks only orchestrate state transitions.
 */
inline bool DecodeGhostResultsResponse(const FString &Json,
                                       FGhostResults &Response) {
  TSharedPtr<FJsonObject> Root;
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid()
             ? false
             : [&]() {
                 Response.SessionId = JsonInterop::OptionalStringFromField(
                     Root, Fields.GhostResultsSessionId);
                 Response.TotalTests =
                     JsonInterop::detail::TryGetNumberAs<int32>(
                         Root, Fields.GhostResultsTotalTests,
                         Response.TotalTests);
                 Response.Passed = JsonInterop::detail::TryGetNumberAs<int32>(
                     Root, Fields.GhostResultsPassed, Response.Passed);
                 Response.Failed = JsonInterop::detail::TryGetNumberAs<int32>(
                     Root, Fields.GhostResultsFailed, Response.Failed);
                 Response.Skipped = JsonInterop::detail::TryGetNumberAs<int32>(
                     Root, Fields.GhostResultsSkipped, Response.Skipped);
                 Response.Duration =
                     JsonInterop::detail::TryGetNumberAs<int64>(
                         Root, Fields.GhostResultsDuration, Response.Duration);
                 Response.Coverage =
                     JsonInterop::detail::TryGetNumberAs<float>(
                         Root, Fields.GhostResultsCoverage, Response.Coverage);
                 const TArray<TSharedPtr<FJsonValue>> *Tests = nullptr;
                 Response.Tests =
                     Root->TryGetArrayField(Fields.GhostResultsTests, Tests) &&
                             Tests
                         ? DecodeGhostTestRecords(*Tests)
                         : TArray<FGhostResultRecord>();
                 const TArray<TSharedPtr<FJsonValue>> *Metrics = nullptr;
                 Response.Metrics =
                     Root->TryGetArrayField(Fields.GhostResultsMetrics,
                                            Metrics) &&
                             Metrics
                         ? DecodeGhostMetrics(*Metrics)
                         : TMap<FString, float>();
                 return true;
               }();
}

/**
 * @fn inline bool DecodeGhostStopResponse(const FString &Json, FGhostStopResponse &Response)
 * User Story: As a Ghost consumer, I need stop metadata decoded through authored fields so completion state never depends on embedded wire names.
 */
inline bool DecodeGhostStopResponse(const FString &Json,
                                    FGhostStopResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  const Endpoints::Configuration::FEndpointConfigurationData &Data =
      Endpoints::Configuration::endpointData();
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.StopStatus = JsonInterop::OptionalStringFromField(
                    Root, Data.Fields.GhostStopStatus),
                Response.StopSessionId = JsonInterop::OptionalStringFromField(
                    Root, Data.Fields.GhostStopSessionId),
                Response.bStopped = Response.StopStatus.Equals(
                    Data.Values.Stopped, ESearchCase::IgnoreCase),
                true);
}

/**
 * @fn inline FGhostHistoryEntry DecodeGhostHistoryEntry(const TSharedPtr<FJsonObject> &Object)
 * User Story: As a Ghost consumer, I need history wire records transformed into stable domain values without compatibility aliases.
 */
inline FGhostHistoryEntry
DecodeGhostHistoryEntry(const TSharedPtr<FJsonObject> &Object) {
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  FGhostHistoryEntry Entry;
  return (Entry.SessionId = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostHistorySessionId),
          Entry.TestSuite = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostHistoryTestSuite),
          Entry.StartedAt = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostHistoryStartedAt),
          Entry.CompletedAt = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostHistoryCompletedAt),
          Entry.Status = JsonInterop::OptionalStringFromField(
              Object, Fields.GhostHistoryStatus),
          Entry.PassRate = JsonInterop::detail::TryGetNumberAs<float>(
              Object, Fields.GhostHistoryPassRate, Entry.PassRate),
          Entry);
}

/**
 * @fn inline bool DecodeGhostHistoryResponse(const FString &Json, FGhostHistoryResponse &Response)
 * User Story: As a Ghost consumer, I need history envelopes normalized with FP collection transforms so missing optional arrays become empty domain collections.
 */
inline bool DecodeGhostHistoryResponse(const FString &Json,
                                       FGhostHistoryResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  const Endpoints::Configuration::FEndpointFieldData &Fields =
      Endpoints::Configuration::endpointData().Fields;
  return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid()
             ? false
             : [&]() {
                 const TArray<TSharedPtr<FJsonValue>> *Sessions = nullptr;
                 Response.Sessions =
                     Root->TryGetArrayField(Fields.GhostHistorySessions,
                                            Sessions) &&
                             Sessions
                         ? func::filter_map_array<TSharedPtr<FJsonValue>,
                                                  FGhostHistoryEntry>(
                               *Sessions,
                               [](const TSharedPtr<FJsonValue> &Value) {
                                 return Value.IsValid() &&
                                        Value->Type == EJson::Object;
                               },
                               [](const TSharedPtr<FJsonValue> &Value) {
                                 return DecodeGhostHistoryEntry(
                                     Value->AsObject());
                               })
                         : TArray<FGhostHistoryEntry>();
                 return true;
               }();
}

} // namespace APISlice::Detail
