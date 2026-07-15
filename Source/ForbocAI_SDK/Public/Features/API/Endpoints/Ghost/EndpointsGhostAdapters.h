#pragma once

#include "Features/API/Endpoints/Soul/EndpointsSoulAdapters.h"

namespace APISlice::Detail {

/**
 * Decodes a ghost-run response into run metadata.
 * User Story: As ghost execution flows, I need run-response decoding so the
 * created session id and initial run status are captured immediately.
 */
inline bool DecodeGhostRunResponse(const FString &Json,
                                   FGhostRunResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.SessionId = Root->GetStringField(TEXT("sessionId")),
                Response.RunStatus = Root->GetStringField(TEXT("runStatus")),
                true);
}

/**
 * Reads an int64 from either numeric or string JSON fields.
 * User Story: As ghost and history codecs, I need tolerant integer parsing so
 * timestamp fields decode correctly even when the API changes wire formats.
 */
inline int64 JsonNumberOrStringToInt64(const TSharedPtr<FJsonObject> &Object,
                                       const FString &FieldName) {
  return (!Object.IsValid() || !Object->HasField(FieldName))
             ? static_cast<int64>(0)
             : [&]() -> int64 {
                 const TSharedPtr<FJsonValue> Value =
                     Object->TryGetField(FieldName);
                 return !Value.IsValid()
                            ? static_cast<int64>(0)
                            : func::or_else(
                                  func::multi_match<EJson, int64>(
                                      Value->Type,
                                      {func::when<EJson, int64>(
                                           func::equals<EJson>(EJson::Number),
                                           [&](const EJson &) -> int64 {
                                             return static_cast<int64>(
                                                 Value->AsNumber());
                                           }),
                                       func::when<EJson, int64>(
                                           func::equals<EJson>(EJson::String),
                                           [&](const EJson &) -> int64 {
                                             return FCString::Atoi64(
                                                 *Value->AsString());
                                           })}),
                                  static_cast<int64>(0));
               }();
}

/**
 * Decodes a ghost-status response into a typed status snapshot.
 * User Story: As ghost execution flows, I need status-response decoding so
 * progress, timing, and error fields can drive polling and UI updates.
 */
inline bool DecodeGhostStatusResponse(const FString &Json,
                                      FGhostStatusResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.GhostSessionId =
                    Root->GetStringField(TEXT("ghostSessionId")),
                Response.getGhostStatus =
                    Root->GetStringField(TEXT("ghostStatus")),
                Response.GhostProgress =
                    JsonInterop::detail::TryGetNumberAs<float>(
                        Root, TEXT("ghostProgress"), Response.GhostProgress),
                Response.GhostStartedAt =
                    JsonNumberOrStringToInt64(Root, TEXT("ghostStartedAt")),
                Response.GhostDuration =
                    JsonInterop::detail::TryGetNumberAs<int32>(
                        Root, TEXT("ghostDuration"), Response.GhostDuration),
                Response.GhostErrors.Empty(),
                [&]() {
                  Root->HasTypedField<EJson::Array>(TEXT("ghostErrors"))
                      ? [&]() {
                          const TArray<TSharedPtr<FJsonValue>> *Values =
                              nullptr;
                          (Root->TryGetArrayField(TEXT("ghostErrors"), Values) &&
                           Values)
                              ? (detail::ExtractGhostErrorsRecursive(
                                     *Values, Response.GhostErrors, 0),
                                 void())
                              : void();
                        }()
                      : (Root->HasField(TEXT("ghostErrors"))
                             ? [&]() {
                                 const TSharedPtr<FJsonValue> Value =
                                     Root->TryGetField(TEXT("ghostErrors"));
                                 Value.IsValid()
                                     ? (func::or_else(
                                           func::multi_match<EJson, bool>(
                                               Value->Type,
                                               {func::when<EJson, bool>(
                                                    func::equals<EJson>(
                                                        EJson::String),
                                                    [&](const EJson &) -> bool {
                                                      !Value->AsString()
                                                               .IsEmpty()
                                                          ? (Response
                                                                 .GhostErrors
                                                                 .Add(Value
                                                                          ->AsString()),
                                                             void())
                                                          : void();
                                                      return true;
                                                    }),
                                                func::when<EJson, bool>(
                                                    func::equals<EJson>(
                                                        EJson::Number),
                                                    [&](const EJson &) -> bool {
                                                      const int32 Count =
                                                          static_cast<int32>(
                                                              Value
                                                                  ->AsNumber());
                                                      Count > 0
                                                          ? (Response
                                                                 .GhostErrors
                                                                 .Add(FString::
                                                                          FromInt(
                                                                              Count)),
                                                             void())
                                                          : void();
                                                      return true;
                                                    })}),
                                           false), void())
                                     : void();
                               }()
                             : void());
                }(),
                true);
}

/**
 * Decodes a ghost-results response into typed result records.
 * User Story: As ghost execution flows, I need results-response decoding so
 * aggregate metrics and per-test outcomes can be inspected in tooling.
 */
inline bool DecodeGhostResultsResponse(const FString &Json,
                                       FGhostResultsResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.ResultsSessionId =
                    Root->GetStringField(TEXT("resultsSessionId")),
                Response.ResultsTotalTests =
                    JsonInterop::detail::TryGetNumberAs<int32>(
                        Root, TEXT("resultsTotalTests"),
                        Response.ResultsTotalTests),
                Response.ResultsPassed =
                    JsonInterop::detail::TryGetNumberAs<int32>(
                        Root, TEXT("resultsPassed"), Response.ResultsPassed),
                Response.ResultsFailed =
                    JsonInterop::detail::TryGetNumberAs<int32>(
                        Root, TEXT("resultsFailed"), Response.ResultsFailed),
                Response.ResultsSkipped =
                    JsonInterop::detail::TryGetNumberAs<int32>(
                        Root, TEXT("resultsSkipped"), Response.ResultsSkipped),
                Response.ResultsDuration =
                    JsonInterop::detail::TryGetNumberAs<int64>(
                        Root, TEXT("resultsDuration"),
                        Response.ResultsDuration),
                Response.ResultsCoverage =
                    JsonInterop::detail::TryGetNumberAs<float>(
                        Root, TEXT("resultsCoverage"),
                        Response.ResultsCoverage),
                Response.ResultsTests.Empty(),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *Tests = nullptr;
                  (Root->TryGetArrayField(TEXT("resultsTests"), Tests) && Tests)
                      ? (detail::ExtractGhostTestRecordsRecursive(
                             *Tests, Response.ResultsTests, 0),
                         void())
                      : void();
                }(),
                Response.ResultsMetrics.Empty(),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *MetricPairs = nullptr;
                  (Root->TryGetArrayField(TEXT("resultsMetrics"), MetricPairs) &&
                   MetricPairs)
                      ? (detail::ExtractGhostMetricPairsRecursive(
                             *MetricPairs, Response.ResultsMetrics, 0),
                         void())
                      : void();
                }(),
                true);
}

/**
 * Decodes a ghost-stop response into typed stop metadata.
 * User Story: As ghost execution flows, I need stop-response decoding so the
 * caller can confirm the target session stopped and record its final state.
 */
inline bool DecodeGhostStopResponse(const FString &Json,
                                    FGhostStopResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.StopStatus = Root->GetStringField(TEXT("stopStatus")),
                Response.StopSessionId =
                    Root->GetStringField(TEXT("stopSessionId")),
                Response.bStopped = Response.StopStatus.Equals(
                    TEXT("stopped"), ESearchCase::IgnoreCase),
                true);
}

/**
 * Recursive ghost history entry extraction definition.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
namespace detail {
inline void ExtractGhostHistoryEntriesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FGhostHistoryEntry> &Out, int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? [&]() {
                 const TSharedPtr<FJsonObject> Session =
                     Source[Index]->AsObject();
                 FGhostHistoryEntry Entry;
                 Entry.SessionId =
                     FieldOrAlias(Session, TEXT("sessionId"),
                                  TEXT("histSessionId"));
                 Entry.TestSuite =
                     FieldOrAlias(Session, TEXT("testSuite"),
                                  TEXT("histTestSuite"));
                 Entry.StartedAt = JsonNumberOrStringToInt64(
                     Session,
                     Session->HasField(TEXT("startedAt"))
                         ? TEXT("startedAt")
                         : TEXT("histStartedAt"));
                 Entry.CompletedAt = JsonNumberOrStringToInt64(
                     Session,
                     Session->HasField(TEXT("completedAt"))
                         ? TEXT("completedAt")
                         : TEXT("histCompletedAt"));
                 Entry.Status =
                     FieldOrAlias(Session, TEXT("status"), TEXT("histStatus"));
                 Entry.PassRate =
                     TryGetPassRate(Session, TEXT("passRate"),
                                   TEXT("histPassRate"));
                 Out.Add(Entry);
               }()
             : void(),
         ExtractGhostHistoryEntriesRecursive(Source, Out, Index + 1), void())
      : void();
}
} // namespace detail

/**
 * Decodes a ghost-history response into typed session history.
 * User Story: As ghost execution flows, I need history-response decoding so
 * prior sessions can be listed with ids, timing, status, and pass-rate data.
 */
inline bool DecodeGhostHistoryResponse(const FString &Json,
                                       FGhostHistoryResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : [&]() -> bool {
                 const TArray<TSharedPtr<FJsonValue>> *Sessions = nullptr;
                 return (!Root->TryGetArrayField(TEXT("sessions"), Sessions) ||
                         !Sessions)
                            ? false
                            : (Response.Sessions.Empty(Sessions->Num()),
                               detail::ExtractGhostHistoryEntriesRecursive(
                                   *Sessions, Response.Sessions, 0),
                               true);
               }();
}

} // namespace APISlice::Detail
