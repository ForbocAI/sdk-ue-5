#pragma once

#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/Config/Resolution/ResolutionAdapters.h"

namespace ConfigSlice {

/** User Story: As automatic endpoint selection, I need one bounded unauthenticated status probe represented as an availability value. @fn inline bool probeApiAvailability(const FConfigConnectionData &Connection) */
inline bool probeApiAvailability(const FConfigConnectionData &Connection) {
  rtk::FetchArgs Args;
  Args.url = normalizeApiUrl(Connection.LocalApiUrl) +
             Connection.StatusPath;
  Args.method = Connection.Methods.Get;
  Args.timeout = Connection.AvailabilityTimeoutMs;
  const double TimeoutSeconds =
      static_cast<double>(Connection.AvailabilityTimeoutMs) /
      static_cast<double>(Connection.MillisecondsPerSecond);
  const func::Either<FString, rtk::QueryReturnValue<FString>> Outcome =
      AsyncAdapters::waitForEither(
          rtk::fetchBaseQuery<FString>()(
              Args, rtk::BaseQueryApi(), rtk::FEmptyPayload{}),
          TimeoutSeconds);
  return func::ematch(
      Outcome, [](const FString &) { return false; },
      [](const rtk::QueryReturnValue<FString> &Result) {
        return !Result.error.hasValue;
      });
}
} // namespace ConfigSlice
