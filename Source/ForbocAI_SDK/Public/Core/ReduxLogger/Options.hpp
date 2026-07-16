#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Misc/DateTime.h"

#include <chrono>
#include <functional>

namespace rtk::logger {

namespace helpers {

/** User Story: As a core redux logger consumer, I need to invoke repeat recursive through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString repeatRecursive(const FString &Value, int32 Times, FString Result) */
inline FString repeatRecursive(const FString &Value, int32 Times,
                               FString Result) {
  return Times <= 0
             ? Result
             : repeatRecursive(Value, Times - 1, Result + Value);
}

/** User Story: As a core redux logger consumer, I need to invoke repeat through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString repeat(const FString &Value, int32 Times) */
inline FString repeat(const FString &Value, int32 Times) {
  return repeatRecursive(Value, Times, FString());
}

/** User Story: As a core redux logger consumer, I need to invoke pad through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString pad(int32 Number, int32 MaxLength) */
inline FString pad(int32 Number, int32 MaxLength) {
  const FString NumberText = FString::FromInt(Number);
  return repeat(TEXT("0"), MaxLength - NumberText.Len()) + NumberText;
}

/** User Story: As a core redux logger consumer, I need to invoke format time through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString formatTime(const FDateTime &Time) */
inline FString formatTime(const FDateTime &Time) {
  return pad(Time.GetHour(), 2) + TEXT(":") + pad(Time.GetMinute(), 2) +
         TEXT(":") + pad(Time.GetSecond(), 2) + TEXT(".") +
         pad(Time.GetMillisecond(), 3);
}

/** User Story: As a core redux logger consumer, I need to invoke timer now through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline double timerNow() */
inline double timerNow() {
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

} // namespace helpers

struct LogEntry {
  double Started = 0.0;
  FDateTime StartedTime;
  FString PrevState;
  AnyAction Action;
  bool bHasError = false;
  FString Error;
  double Took = 0.0;
  FString NextState;
};

namespace detail {

/** User Story: As a core redux logger consumer, I need to invoke format action through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline FString formatAction(const AnyAction &Action) */
inline FString formatAction(const AnyAction &Action) {
  const FString Payload = Action.describePayload();
  return Payload == TEXT("<none>")
             ? FString::Printf(TEXT("{ type: \"%s\" }"), *Action.Type)
             : FString::Printf(TEXT("{ type: \"%s\", payload: %s }"),
                               *Action.Type, *Payload);
}

/** User Story: As a core redux logger consumer, I need to invoke payload list through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline TArray<FString> payloadList(const FString &First) */
inline TArray<FString> payloadList(const FString &First) {
  TArray<FString> Payload;
  Payload.Add(First);
  return Payload;
}

/** User Story: As a core redux logger consumer, I need to invoke payload list through a stable signature so the core redux logger workflow remains explicit and composable. @fn inline TArray<FString> payloadList(const FString &First, const FString &Second) */
inline TArray<FString> payloadList(const FString &First,
                                   const FString &Second) {
  TArray<FString> Payload;
  Payload.Add(First);
  Payload.Add(Second);
  return Payload;
}

} // namespace detail

template <typename State> struct LoggerColors {
  std::function<FString(const FString &)> Title =
      [](const FString &) { return FString(TEXT("inherit")); };
  std::function<FString(const FString &)> PrevState =
      [](const FString &) { return FString(TEXT("#9E9E9E")); };
  std::function<FString(const FString &)> Action =
      [](const FString &) { return FString(TEXT("#03A9F4")); };
  std::function<FString(const FString &)> NextState =
      [](const FString &) { return FString(TEXT("#4CAF50")); };
  std::function<FString(const FString &, const FString &)> Error =
      [](const FString &, const FString &) {
        return FString(TEXT("#F20404"));
      };
};

using LevelCallback =
    std::function<FString(const AnyAction &, const TArray<FString> &)>;

template <typename State> struct ReduxLoggerOptions {
  FString Level = TEXT("log");
  TMap<FString, FString> LevelByType;
  TMap<FString, LevelCallback> LevelByTypeFn;
  std::function<FString(const AnyAction &)> LevelFn;
  std::function<void(const FString &)> Logger;
  bool bLoggerAvailable = true;
  bool bLogErrors = true;
  bool bCollapsed = false;
  std::function<bool(const std::function<FString()> &, const AnyAction &,
                     const LogEntry &)>
      CollapsedFn;
  std::function<bool(const std::function<const State &()> &, const AnyAction &)>
      Predicate;
  bool bDuration = false;
  bool bTimestamp = true;
  std::function<FString(const State &)> StateTransformer =
      [](const State &Value) {
        return payload_debug::DebugPayloadString(Value);
      };
  std::function<AnyAction(const AnyAction &)> ActionTransformer =
      [](const AnyAction &Action) { return Action; };
  std::function<FString(const AnyAction &)> ActionFormatter =
      [](const AnyAction &Action) { return detail::formatAction(Action); };
  std::function<FString(const FString &)> ErrorTransformer =
      [](const FString &Error) { return Error; };
  LoggerColors<State> Colors;
  bool bDiff = false;
  std::function<bool(const std::function<const State &()> &, const AnyAction &)>
      DiffPredicate;
  std::function<FString(const AnyAction &, const FString &, double)>
      TitleFormatter;
  bool bWithTrace = false;
  std::function<FString(const State &)> Transformer;
  std::function<void(const FString &, const FString &,
                     const std::function<void(const FString &)> &, bool)>
      DiffLogger;
};

} // namespace rtk::logger
