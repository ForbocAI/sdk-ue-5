#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

#include "Systems/Errors/ErrorsAdapters.h"
#include "Systems/API/APIApi.h"
#include "Entities/Ghost/GhostSlice.h"
#include "Entities/Config/ConfigSelectors.h"

namespace rtk {

/**
 * Ghost thunks (mirrors TS ghostSlice.ts)
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 * @fn inline ThunkAction<FGhostRunResponse, FRuntimeState> startGhostThunk(const FGhostConfig &Config)
 */

inline ThunkAction<FGhostRunResponse, FRuntimeState>
startGhostThunk(const FGhostConfig &Config) {
  return [Config](std::function<AnyAction(const AnyAction &)> Dispatch,
                  std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostRunResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        ConfigSelectors::selectApiUrl(GetState()),
        ConfigSelectors::selectApiKey(GetState()));
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

/** User Story: As a features ghost consumer, I need to invoke get ghost status thunk through a stable signature so the features ghost workflow remains explicit and composable. @fn inline ThunkAction<FGhostStatus, FRuntimeState> getGhostStatusThunk(const FString &SessionId) */
inline ThunkAction<FGhostStatus, FRuntimeState>
getGhostStatusThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostStatus> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        ConfigSelectors::selectApiUrl(GetState()),
        ConfigSelectors::selectApiKey(GetState()));
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostStatus>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostStatus, FGhostStatus>(
        APISlice::Endpoints::getGhostStatus(SessionId)(Dispatch, GetState),
        [Dispatch](const FGhostStatus &Response) {
          Dispatch(GhostSlice::Actions::ghostSessionProgress(
              Response.SessionId, Response.Status, Response.Progress));
          return detail::ResolveAsync(Response);
        });
  };
}

/** User Story: As a features ghost consumer, I need to invoke get ghost results thunk through a stable signature so the features ghost workflow remains explicit and composable. @fn inline ThunkAction<FGhostResults, FRuntimeState> getGhostResultsThunk(const FString &SessionId) */
inline ThunkAction<FGhostResults, FRuntimeState>
getGhostResultsThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostResults> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        ConfigSelectors::selectApiUrl(GetState()),
        ConfigSelectors::selectApiKey(GetState()));
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostResults>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostResults, FGhostResults>(
        APISlice::Endpoints::getGhostResults(SessionId)(Dispatch, GetState),
        [Dispatch](const FGhostResults &Response) {
          FGhostTestReport Report;
          Report.SessionId = Response.SessionId;
          Report.TotalTests = Response.TotalTests;
          Report.PassedTests = Response.Passed;
          Report.FailedTests = Response.Failed;
          Report.SkippedTests = Response.Skipped;
          Report.Duration = Response.Duration;
          Report.Coverage = Response.Coverage;
          Report.Metrics = Response.Metrics;
          Report.SuccessRate =
              Response.TotalTests > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA
                  ? static_cast<float>(Response.Passed) /
                        static_cast<float>(Response.TotalTests)
                  : FORBOCAI_SDK_AUTHORED_NUMBERV75F40683FBFF;
          Report.Summary = FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV91268A5A2613),
                                           Response.Passed,
                                           Response.TotalTests);

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
                     apply(Tests, Out, Idx + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4), void());
            }
          };
          AddResults::apply(Response.Tests, Report.Results, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);

          Dispatch(GhostSlice::Actions::ghostSessionCompleted(Report));
          return detail::ResolveAsync(Response);
        });
  };
}

/** User Story: As a features ghost consumer, I need to invoke stop ghost thunk through a stable signature so the features ghost workflow remains explicit and composable. @fn inline ThunkAction<FGhostStopResponse, FRuntimeState> stopGhostThunk(const FString &SessionId) */
inline ThunkAction<FGhostStopResponse, FRuntimeState>
stopGhostThunk(const FString &SessionId) {
  return [SessionId](std::function<AnyAction(const AnyAction &)> Dispatch,
                     std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FGhostStopResponse> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        ConfigSelectors::selectApiUrl(GetState()),
        ConfigSelectors::selectApiKey(GetState()));
    return ApiKeyError.hasValue
        ? detail::RejectAsync<FGhostStopResponse>(ApiKeyError.value)
        : func::AsyncChain::then<FGhostStopResponse, FGhostStopResponse>(
        APISlice::Endpoints::postGhostStop(SessionId)(Dispatch, GetState),
        [Dispatch, SessionId](const FGhostStopResponse &Response) {
          const bool bStopped =
              Response.bStopped ||
              Response.StopStatus.Equals(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6F0C98AB7582),
                                         ESearchCase::IgnoreCase);
          bStopped
              ? (Dispatch(GhostSlice::Actions::ghostSessionProgress(
                     Response.StopSessionId.IsEmpty() ? SessionId
                                                      : Response.StopSessionId,
                     Response.StopStatus.IsEmpty() ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6F0C98AB7582)
                                                   : Response.StopStatus,
                     FORBOCAI_SDK_AUTHORED_NUMBERV8B65CDBB20CA)),
                 void())
              : void();
          return detail::ResolveAsync(Response);
        });
  };
}

/**
 * User Story: As a features ghost consumer, I need to invoke get ghost history thunk through a stable signature so the features ghost workflow remains explicit and composable.
 * @fn inline ThunkAction<TArray<FGhostHistoryEntry>, FRuntimeState> getGhostHistoryThunk(int32 Limit = FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F)
 */
inline ThunkAction<TArray<FGhostHistoryEntry>, FRuntimeState>
getGhostHistoryThunk(int32 Limit = FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F) {
  return [Limit](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FGhostHistoryEntry>> {
    const auto ApiKeyError = Errors::requireApiKeyGuidance(
        ConfigSelectors::selectApiUrl(GetState()),
        ConfigSelectors::selectApiKey(GetState()));
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
