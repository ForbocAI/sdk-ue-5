#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

#include "Errors.h"
#include "Features/API/APIApi.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Ghost/Local/GhostLocalThunks.h"
#include "RuntimeConfig.h"

namespace rtk {

/**
 * Ghost thunks (mirrors TS ghostSlice.ts)
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

inline ThunkAction<FGhostRunResponse, FRuntimeState>
startGhostThunk(const FGhostConfig &Config) {
  return [Config](std::function<AnyAction(const AnyAction &)> Dispatch,
                  std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostRunResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostRunResponse>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostRunResponse, FGhostRunResponse>(
        APISlice::Endpoints::postGhostRun(Config)(Dispatch, GetState),
        [Dispatch](const FGhostRunResponse &Response) {
          Dispatch(GhostSlice::Actions::ghostSessionStarted(
              Response.SessionId, Response.RunStatus));
          return detail::ResolveAsync(Response);
        });
  };
}

inline ThunkAction<FGhostStatusResponse, FRuntimeState>
getGhostStatusThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostStatusResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostStatusResponse>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostStatusResponse, FGhostStatusResponse>(
        APISlice::Endpoints::getGhostStatus(SessionId)(Dispatch, GetState),
        [Dispatch](const FGhostStatusResponse &Response) {
          Dispatch(GhostSlice::Actions::ghostSessionProgress(
              Response.GhostSessionId.IsEmpty() ? TEXT("") : Response.GhostSessionId,
              Response.getGhostStatus, Response.GhostProgress));
          return detail::ResolveAsync(Response);
        });
  };
}

inline ThunkAction<FGhostResultsResponse, FRuntimeState>
getGhostResultsThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostResultsResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostResultsResponse>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostResultsResponse,
                                  FGhostResultsResponse>(
        APISlice::Endpoints::getGhostResults(SessionId)(Dispatch, GetState),
        [Dispatch](const FGhostResultsResponse &Response) {
          FGhostTestReport Report;
          Report.SessionId = Response.ResultsSessionId;
          Report.TotalTests = Response.ResultsTotalTests;
          Report.PassedTests = Response.ResultsPassed;
          Report.FailedTests = Response.ResultsFailed;
          Report.SkippedTests = Response.ResultsSkipped;
          Report.Duration = Response.ResultsDuration;
          Report.Coverage = Response.ResultsCoverage;
          Report.Metrics = Response.ResultsMetrics;
          Report.SuccessRate =
              Response.ResultsTotalTests > 0
                  ? static_cast<float>(Response.ResultsPassed) /
                        static_cast<float>(Response.ResultsTotalTests)
                  : 0.0f;
          Report.Summary = FString::Printf(TEXT("%d/%d passed"),
                                           Response.ResultsPassed,
                                           Response.ResultsTotalTests);

          struct AddResults {
            static FGhostTestResult makeResult(
                const FGhostResultRecord &Record) {
              FGhostTestResult R;
              R.Scenario = Record.TestName;
              R.bPassed = Record.bTestPassed;
              R.Duration = Record.TestDuration;
              R.ErrorMessage = Record.TestError;
              R.Screenshot = Record.TestScreenshot;
              return R;
            }
            static void apply(
                const TArray<FGhostResultRecord> &Tests,
                TArray<FGhostTestResult> &Out,
                int32 Idx) {
              Idx >= Tests.Num()
                  ? void()
                  : (Out.Add(makeResult(Tests[Idx])),
                     apply(Tests, Out, Idx + 1), void());
            }
          };
          AddResults::apply(Response.ResultsTests, Report.Results, 0);

          Dispatch(GhostSlice::Actions::ghostSessionCompleted(Report));
          return detail::ResolveAsync(Response);
        });
  };
}

inline ThunkAction<FGhostStopResponse, FRuntimeState>
stopGhostThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostStopResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostStopResponse>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostStopResponse, FGhostStopResponse>(
        APISlice::Endpoints::postGhostStop(SessionId)(Dispatch, GetState),
        [Dispatch, SessionId](const FGhostStopResponse &Response) {
          const bool bStopped =
              Response.bStopped ||
              Response.StopStatus.Equals(TEXT("stopped"),
                                         ESearchCase::IgnoreCase);
          bStopped
              ? (Dispatch(GhostSlice::Actions::ghostSessionProgress(
                     Response.StopSessionId.IsEmpty() ? SessionId
                                                      : Response.StopSessionId,
                     Response.StopStatus.IsEmpty() ? TEXT("stopped")
                                                   : Response.StopStatus,
                     1.0f)),
                 void())
              : void();
          return detail::ResolveAsync(Response);
        });
  };
}

inline ThunkAction<TArray<FGhostHistoryEntry>, FRuntimeState>
getGhostHistoryThunk(int32 Limit = 10) {
  return [Limit](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FGhostHistoryEntry>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        SDKConfig::GetApiUrl(), SDKConfig::GetApiKey());
    return ApiKeyError.hasValue
        ? detail::RejectAsync<TArray<FGhostHistoryEntry>>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostHistoryResponse,
                                  TArray<FGhostHistoryEntry>>(
        APISlice::Endpoints::getGhostHistory(Limit)(Dispatch, GetState),
        [Dispatch](const FGhostHistoryResponse &Response) {
          Dispatch(GhostSlice::Actions::ghostHistoryLoaded(Response.Sessions));
          return detail::ResolveAsync(Response.Sessions);
        });
  };
}

} // namespace rtk
