#pragma once
#ifndef REDUX_LOGGER_HPP
#define REDUX_LOGGER_HPP

#include "CoreMinimal.h"
#include "Misc/DateTime.h"
#include "rtk.hpp"

#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/**
 * C++11/Unreal port of LogRocket/redux-logger.
 *
 * Upstream source map:
 *   helpers.js  -> helpers::{repeat, pad, formatTime, timerNow}
 *   defaults.js -> ReduxLoggerOptions and LoggerColors
 *   core.js     -> detail::{getLogLevel, defaultTitleFormatter, printBuffer}
 *   diff.js     -> detail::{diffKindText, diffStyle, fallbackDiffLogger}
 *   index.js    -> createLogger and defaultLogger
 *
 * Port notes:
 *   - JavaScript console groups and CSS `%c` styling are represented as plain
 *     FString log lines because UE_LOG is a text sink.
 *   - JavaScript can pass arbitrary objects to console.log. This port keeps
 *     transformed state as text and transforms actions as AnyAction values with
 *     a separate ActionFormatter for text output.
 *   - deep-diff is not bundled. Set DiffLogger to provide field-level diffs.
 */
namespace rtk {
namespace logger {

/* =============================================================================
 * helpers.js
 * ========================================================================== */
namespace helpers {

// export const repeat = (str, times) => (new Array(times + 1)).join(str);
inline FString repeatRecursive(const FString &Str, int32 Times,
                               FString Acc) {
  return Times <= 0 ? Acc
                    : repeatRecursive(Str, Times - 1, Acc + Str);
}

inline FString repeat(const FString &Str, int32 Times) {
  return repeatRecursive(Str, Times, FString());
}

// export const pad = (num, maxLength) => repeat('0', maxLength - num.toString().length) + num;
inline FString pad(int32 Num, int32 MaxLength) {
  const FString NumStr = FString::FromInt(Num);
  return repeat(TEXT("0"), MaxLength - NumStr.Len()) + NumStr;
}

// export const formatTime = time => `${HH}:${MM}:${SS}.${mmm}`;
inline FString formatTime(const FDateTime &Time) {
  return pad(Time.GetHour(), 2) + TEXT(":") + pad(Time.GetMinute(), 2) +
         TEXT(":") + pad(Time.GetSecond(), 2) + TEXT(".") +
         pad(Time.GetMillisecond(), 3);
}

// Use performance API if it's available in order to get better precision.
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

inline FString formatAction(const AnyAction &Action) {
  const FString Payload = Action.describePayload();
  return Payload == TEXT("<none>")
             ? FString::Printf(TEXT("{ type: \"%s\" }"), *Action.Type)
             : FString::Printf(TEXT("{ type: \"%s\", payload: %s }"),
                               *Action.Type, *Payload);
}

inline TArray<FString> payloadList(const FString &A) {
  TArray<FString> Payload;
  Payload.Add(A);
  return Payload;
}

inline TArray<FString> payloadList(const FString &A, const FString &B) {
  TArray<FString> Payload;
  Payload.Add(A);
  Payload.Add(B);
  return Payload;
}

} // namespace detail

/* =============================================================================
 * defaults.js
 * ========================================================================== */

template <typename State> struct LoggerColors {
  // title: () => 'inherit'
  std::function<FString(const FString & /*formattedAction*/)> Title =
      [](const FString &) { return FString(TEXT("inherit")); };

  // prevState: () => '#9E9E9E'
  std::function<FString(const FString & /*prevState*/)> PrevState =
      [](const FString &) { return FString(TEXT("#9E9E9E")); };

  // action: () => '#03A9F4'
  std::function<FString(const FString & /*formattedAction*/)> Action =
      [](const FString &) { return FString(TEXT("#03A9F4")); };

  // nextState: () => '#4CAF50'
  std::function<FString(const FString & /*nextState*/)> NextState =
      [](const FString &) { return FString(TEXT("#4CAF50")); };

  // error: () => '#F20404'
  std::function<FString(const FString & /*error*/, const FString & /*prevState*/)>
      Error = [](const FString &, const FString &) {
        return FString(TEXT("#F20404"));
      };
};

using LevelCallback =
    std::function<FString(const AnyAction & /*formattedAction*/,
                          const TArray<FString> & /*payload*/)> ;

template <typename State> struct ReduxLoggerOptions {
  // level: 'log'
  FString Level = TEXT("log");

  // level as an object: level[type]
  TMap<FString, FString> LevelByType;

  // level as an object with function values: level[type](...payload)
  TMap<FString, LevelCallback> LevelByTypeFn;

  // level as a function: level(action)
  std::function<FString(const AnyAction &)> LevelFn;

  // logger: console. Empty Logger falls back to UE_LOG when bLoggerAvailable is true.
  std::function<void(const FString &)> Logger;

  // Return if 'console' object is not defined.
  bool bLoggerAvailable = true;

  // logErrors: true
  bool bLogErrors = true;

  // collapsed: undefined
  bool bCollapsed = false;

  // collapsed(() => nextState, action, logEntry)
  std::function<bool(const std::function<FString()> &, const AnyAction &,
                     const LogEntry &)>
      CollapsedFn;

  // predicate(getState, action)
  std::function<bool(const std::function<State()> &, const AnyAction &)> Predicate;

  // duration: false
  bool bDuration = false;

  // timestamp: true
  bool bTimestamp = true;

  // stateTransformer: state => state
  std::function<FString(const State &)> StateTransformer =
      [](const State &S) { return payload_debug::DebugPayloadString(S); };

  // actionTransformer: action => action
  std::function<AnyAction(const AnyAction &)> ActionTransformer =
      [](const AnyAction &A) { return A; };

  // Text rendering layer for UE_LOG; JavaScript delegates this to console.log.
  std::function<FString(const AnyAction &)> ActionFormatter =
      [](const AnyAction &A) { return detail::formatAction(A); };

  // errorTransformer: error => error
  std::function<FString(const FString &)> ErrorTransformer =
      [](const FString &E) { return E; };

  // colors: { title, prevState, action, nextState, error }
  LoggerColors<State> Colors;

  // diff: false
  bool bDiff = false;

  // diffPredicate: undefined
  std::function<bool(const std::function<State()> &, const AnyAction &)> DiffPredicate;

  // titleFormatter = defaultTitleFormatter(options)
  std::function<FString(const AnyAction &, const FString &, double)> TitleFormatter;

  // logger.withTrace
  bool bWithTrace = false;

  // Deprecated options: transformer: undefined
  std::function<FString(const State &)> Transformer;

  // diffLogger(prevState, newState, logger, isCollapsed)
  std::function<void(const FString &, const FString &,
                     const std::function<void(const FString &)> &, bool)>
      DiffLogger;
};

/* =============================================================================
 * core.js
 * ========================================================================== */
namespace detail {

template <typename State>
bool hasObjectLevel(const ReduxLoggerOptions<State> &Options) {
  return Options.LevelByType.Num() > 0 || Options.LevelByTypeFn.Num() > 0;
}

inline FString defaultLevelValue(const FString *FoundLevel) {
  return FoundLevel ? *FoundLevel : FString();
}

inline FString invokeLevelCallback(const LevelCallback *FoundFn,
                                   const AnyAction &FormattedAction,
                                   const TArray<FString> &Payload,
                                   const FString *FoundLevel) {
  return (FoundFn && *FoundFn) ? (*FoundFn)(FormattedAction, Payload)
                               : defaultLevelValue(FoundLevel);
}

template <typename State>
FString getObjectLogLevel(const ReduxLoggerOptions<State> &Options,
                          const AnyAction &FormattedAction,
                          const TArray<FString> &Payload,
                          const FString &Type) {
  return invokeLevelCallback(Options.LevelByTypeFn.Find(Type),
                             FormattedAction, Payload,
                             Options.LevelByType.Find(Type));
}

/**
 * Get log level string based on supplied params.
 *
 * JavaScript switch:
 *   object   -> level[type](...payload) or level[type]
 *   function -> level(action)
 *   default  -> level
 */
template <typename State>
FString getLogLevel(const ReduxLoggerOptions<State> &Options,
                    const AnyAction &FormattedAction,
                    const TArray<FString> &Payload, const FString &Type) {
  return hasObjectLevel(Options)
             ? getObjectLogLevel(Options, FormattedAction, Payload, Type)
             : (Options.LevelFn ? Options.LevelFn(FormattedAction)
                                : Options.Level);
}

template <typename State>
FString transformState(const ReduxLoggerOptions<State> &Options,
                       const State &StateValue) {
  return Options.Transformer ? Options.Transformer(StateValue)
                             : Options.StateTransformer(StateValue);
}

template <typename State>
FString formatActionText(const ReduxLoggerOptions<State> &Options,
                         const AnyAction &Action) {
  return Options.ActionFormatter ? Options.ActionFormatter(Action)
                                 : detail::formatAction(Action);
}

// defaultTitleFormatter(options)(action, time, took)
template <typename State>
FString defaultTitleFormatter(const ReduxLoggerOptions<State> &Options,
                              const AnyAction &Action, const FString &Time,
                              double Took) {
  return TEXT("action ") + Action.Type +
         (Options.bTimestamp
              ? FString::Printf(TEXT(" @ %s"), *Time)
              : FString()) +
         (Options.bDuration
              ? FString::Printf(TEXT(" (in %.2f ms)"), Took)
              : FString());
}

inline void logRow(const std::function<void(const FString &)> &Logger,
                   const FString &Level, const FString &Label,
                   const FString &Value) {
  !Level.IsEmpty()
      ? (Logger(FString::Printf(TEXT("  %s %s"), *Label, *Value)), void())
      : void();
}

inline FString diffKindText(const FString &Kind) {
  return Kind == TEXT("E")   ? FString(TEXT("CHANGED:"))
         : Kind == TEXT("N") ? FString(TEXT("ADDED:"))
         : Kind == TEXT("D") ? FString(TEXT("DELETED:"))
         : Kind == TEXT("A") ? FString(TEXT("ARRAY:"))
                              : FString();
}

inline FString diffStyle(const FString &Kind) {
  const FString Color =
      Kind == TEXT("N")   ? FString(TEXT("#4CAF50"))
      : Kind == TEXT("D") ? FString(TEXT("#F44336"))
                           : FString(TEXT("#2196F3"));
  return FString::Printf(TEXT("color: %s; font-weight: bold"), *Color);
}

inline void fallbackDiffLogger(
    const FString &PrevState, const FString &NextState,
    const std::function<void(const FString &)> &Logger, bool bIsCollapsed) {
  Logger(bIsCollapsed ? TEXT("  diff (collapsed)") : TEXT("  diff"));
  Logger(PrevState == NextState
             ? TEXT("    \u2014\u2014 no diff \u2014\u2014")
             : TEXT("    CHANGED: <state text changed; set DiffLogger for field-level diff>"));
  Logger(TEXT("  \u2014\u2014 diff end \u2014\u2014"));
}

inline FString nextStateForBufferKey(const std::vector<LogEntry> &Buffer,
                                     size_t Key, const LogEntry &Entry) {
  return Key + 1 < Buffer.size() ? Buffer[Key + 1].PrevState
                                 : Entry.NextState;
}

inline double tookForBufferKey(const std::vector<LogEntry> &Buffer, size_t Key,
                               const LogEntry &Entry) {
  return Key + 1 < Buffer.size() ? Buffer[Key + 1].Started - Entry.Started
                                 : Entry.Took;
}

inline bool resolveCollapsed(
    const std::function<bool(const std::function<FString()> &,
                             const AnyAction &, const LogEntry &)> &CollapsedFn,
    bool bCollapsed, const std::function<FString()> &GetNextState,
    const AnyAction &FormattedAction, const LogEntry &Entry) {
  return CollapsedFn ? CollapsedFn(GetNextState, FormattedAction, Entry)
                     : bCollapsed;
}

inline void logTraceWhen(const std::function<void(const FString &)> &Logger,
                         bool bWithTrace) {
  bWithTrace ? (Logger(TEXT("  TRACE")),
                Logger(TEXT("    <stack trace unavailable in UE_LOG sink>")),
                void())
             : void();
}

inline void logErrorWhen(const std::function<void(const FString &)> &Logger,
                         const FString &ErrorLevel, const LogEntry &Entry) {
  Entry.bHasError ? logRow(Logger, ErrorLevel, TEXT("error     "),
                           Entry.Error)
                  : void();
}

template <typename State>
void logDiffWhen(const ReduxLoggerOptions<State> &Options,
                 const std::function<void(const FString &)> &Logger,
                 const FString &PrevState, const FString &NextState,
                 bool bIsCollapsed) {
  Options.bDiff
      ? (Options.DiffLogger
             ? Options.DiffLogger(PrevState, NextState, Logger, bIsCollapsed)
             : fallbackDiffLogger(PrevState, NextState, Logger, bIsCollapsed))
      : void();
}

template <typename State>
void printBufferEntry(const std::vector<LogEntry> &Buffer, size_t Key,
                      const ReduxLoggerOptions<State> &Options,
                      const std::function<void(const FString &)> &Logger) {
  const LogEntry &Entry = Buffer[Key];
  const double Took = tookForBufferKey(Buffer, Key, Entry);
  const FString NextState = nextStateForBufferKey(Buffer, Key, Entry);
  const AnyAction FormattedAction =
      Options.ActionTransformer ? Options.ActionTransformer(Entry.Action)
                                : Entry.Action;
  const FString FormattedActionText =
      formatActionText(Options, FormattedAction);
  const std::function<FString()> GetNextState = [&NextState]() {
    return NextState;
  };
  const bool bIsCollapsed =
      resolveCollapsed(Options.CollapsedFn, Options.bCollapsed, GetNextState,
                       FormattedAction, Entry);
  const FString FormattedTime = helpers::formatTime(Entry.StartedTime);
  const FString TitleCSS =
      Options.Colors.Title
          ? FString::Printf(TEXT("color: %s;"),
                            *Options.Colors.Title(FormattedActionText))
          : FString();
  (void)TitleCSS;

  const FString Title =
      Options.TitleFormatter
          ? Options.TitleFormatter(FormattedAction, FormattedTime, Took)
          : defaultTitleFormatter(Options, FormattedAction, FormattedTime,
                                  Took);

  Logger(Title);

  const FString PrevStateLevel = getLogLevel(
      Options, FormattedAction, payloadList(Entry.PrevState),
      TEXT("prevState"));
  const FString ActionLevel = getLogLevel(
      Options, FormattedAction, payloadList(FormattedActionText),
      TEXT("action"));
  const FString ErrorLevel =
      getLogLevel(Options, FormattedAction,
                  payloadList(Entry.Error, Entry.PrevState), TEXT("error"));
  const FString NextStateLevel = getLogLevel(
      Options, FormattedAction, payloadList(NextState), TEXT("nextState"));

  logRow(Logger, PrevStateLevel, TEXT("prev state"), Entry.PrevState);
  logRow(Logger, ActionLevel, TEXT("action    "), FormattedActionText);
  logErrorWhen(Logger, ErrorLevel, Entry);
  logRow(Logger, NextStateLevel, TEXT("next state"), NextState);
  logTraceWhen(Logger, Options.bWithTrace);
  logDiffWhen(Options, Logger, Entry.PrevState, NextState, bIsCollapsed);
}

template <typename State>
void printBufferRecursive(const std::vector<LogEntry> &Buffer, size_t Key,
                          const ReduxLoggerOptions<State> &Options,
                          const std::function<void(const FString &)> &Logger) {
  Key >= Buffer.size()
      ? void()
      : (printBufferEntry(Buffer, Key, Options, Logger),
         printBufferRecursive(Buffer, Key + 1, Options, Logger));
}

/**
 * printBuffer(buffer, options)
 *
 * Mirrors core.js:
 *   - derive nextState/took from the next buffered entry when present
 *   - apply actionTransformer
 *   - resolve collapsed
 *   - build title and render group/log rows
 *   - emit optional trace and diff
 *   - end group, falling back to a log line when grouping is unavailable
 */
template <typename State>
void printBuffer(const std::vector<LogEntry> &Buffer,
                 const ReduxLoggerOptions<State> &Options,
                 const std::function<void(const FString &)> &Logger) {
  printBufferRecursive(Buffer, 0, Options, Logger);
}

struct LoggedDispatchResult {
  AnyAction ReturnedValue;
  bool bHasError;
  FString Error;
};

inline LoggedDispatchResult loggedSuccess(const AnyAction &ReturnedValue) {
  return LoggedDispatchResult{ReturnedValue, false, FString()};
}

inline LoggedDispatchResult loggedFailure(const FString &Error) {
  return LoggedDispatchResult{AnyAction(), true, Error};
}

template <typename State>
LoggedDispatchResult
dispatchWithErrorCapture(const ReduxLoggerOptions<State> &Options,
                         Dispatcher Next, const AnyAction &Action) {
#if PLATFORM_EXCEPTIONS_DISABLED
  (void)Options;
  return loggedSuccess(Next(Action));
#else
  try {
    return loggedSuccess(Next(Action));
  } catch (const std::exception &E) {
    return loggedFailure(
        Options.ErrorTransformer(FString(UTF8_TO_TCHAR(E.what()))));
  } catch (...) {
    return loggedFailure(Options.ErrorTransformer(TEXT("<unknown error>")));
  }
#endif
}

inline LoggedDispatchResult dispatchWithoutErrorCapture(
    Dispatcher Next, const AnyAction &Action) {
  return loggedSuccess(Next(Action));
}

template <typename State>
LoggedDispatchResult dispatchNext(const ReduxLoggerOptions<State> &Options,
                                  Dispatcher Next,
                                  const AnyAction &Action) {
  return Options.bLogErrors ? dispatchWithErrorCapture(Options, Next, Action)
                            : dispatchWithoutErrorCapture(Next, Action);
}

inline LogEntry applyDispatchResult(LogEntry Entry,
                                    const LoggedDispatchResult &Result) {
  Entry.bHasError = Result.bHasError;
  Entry.Error = Result.Error;
  return Entry;
}

template <typename State>
LogEntry startLogEntry(const ReduxLoggerOptions<State> &Options,
                       const MiddlewareApi<State> &Api,
                       const AnyAction &Action) {
  LogEntry Entry;
  Entry.Started = helpers::timerNow();
  Entry.StartedTime = FDateTime::Now();
  Entry.PrevState = transformState(Options, Api.getState());
  Entry.Action = Action;
  return Entry;
}

template <typename State>
LogEntry finishLogEntry(const ReduxLoggerOptions<State> &Options,
                        const MiddlewareApi<State> &Api, LogEntry Entry) {
  Entry.Took = helpers::timerNow() - Entry.Started;
  Entry.NextState = transformState(Options, Api.getState());
  return Entry;
}

template <typename State>
ReduxLoggerOptions<State>
withResolvedDiff(const ReduxLoggerOptions<State> &Options,
                 const std::function<State()> &GetState,
                 const AnyAction &Action) {
  ReduxLoggerOptions<State> PrintOptions = Options;
  PrintOptions.bDiff = (Options.bDiff && Options.DiffPredicate)
                           ? Options.DiffPredicate(GetState, Action)
                           : Options.bDiff;
  return PrintOptions;
}

inline void rethrowLoggedError(const LogEntry &Entry) {
#if !PLATFORM_EXCEPTIONS_DISABLED
  Entry.bHasError
      ? throw std::runtime_error(std::string(TCHAR_TO_UTF8(*Entry.Error)))
      : void();
#else
  (void)Entry;
#endif
}

template <typename State>
AnyAction dispatchWithLogging(
    const ReduxLoggerOptions<State> &Options,
    const std::function<void(const FString &)> &Logger,
    const std::shared_ptr<std::vector<LogEntry>> &LogBuffer,
    const MiddlewareApi<State> &Api, Dispatcher Next,
    const AnyAction &Action, const std::function<State()> &GetState) {
  const LogEntry StartedEntry = startLogEntry(Options, Api, Action);
  const LoggedDispatchResult DispatchResult =
      dispatchNext(Options, Next, Action);
  const LogEntry Entry = finishLogEntry(
      Options, Api, applyDispatchResult(StartedEntry, DispatchResult));
  const ReduxLoggerOptions<State> PrintOptions =
      withResolvedDiff(Options, GetState, Action);

  LogBuffer->push_back(Entry);
  printBuffer(*LogBuffer, PrintOptions, Logger);
  LogBuffer->clear();
  rethrowLoggedError(Entry);

  return DispatchResult.ReturnedValue;
}

template <typename State>
AnyAction dispatchOrSkip(
    const ReduxLoggerOptions<State> &Options,
    const std::function<void(const FString &)> &Logger,
    const std::shared_ptr<std::vector<LogEntry>> &LogBuffer,
    const MiddlewareApi<State> &Api, Dispatcher Next,
    const AnyAction &Action) {
  const std::function<State()> GetState = [Api]() -> State {
    return Api.getState();
  };
  return (Options.Predicate && !Options.Predicate(GetState, Action))
             ? Next(Action)
             : dispatchWithLogging(Options, Logger, LogBuffer, Api, Next,
                                   Action, GetState);
}

template <typename State> Middleware<State> passThroughMiddleware() {
  return [](const MiddlewareApi<State> &)
             -> std::function<Dispatcher(Dispatcher)> {
    return [](Dispatcher Next) -> Dispatcher {
      return [Next](const AnyAction &Action) -> AnyAction {
        return Next(Action);
      };
    };
  };
}

template <typename State>
std::function<void(const FString &)>
resolveLogger(const ReduxLoggerOptions<State> &Options) {
  return Options.Logger ? Options.Logger
                        : [](const FString &Msg) {
                            UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
                          };
}

template <typename State>
Middleware<State>
activeLoggerMiddleware(const ReduxLoggerOptions<State> &Options) {
  const std::function<void(const FString &)> Logger = resolveLogger(Options);
  auto LogBuffer = std::make_shared<std::vector<LogEntry>>();

  return [Options, Logger,
          LogBuffer](const MiddlewareApi<State> &Api)
             -> std::function<Dispatcher(Dispatcher)> {
    return [Options, Logger, LogBuffer, Api](Dispatcher Next) -> Dispatcher {
      return [Options, Logger, LogBuffer, Api,
              Next](const AnyAction &Action) -> AnyAction {
        return dispatchOrSkip(Options, Logger, LogBuffer, Api, Next, Action);
      };
    };
  };
}

} // namespace detail

/* =============================================================================
 * index.js
 * ========================================================================== */

/**
 * Creates logger with following options.
 *
 * JavaScript middleware flow:
 *   create loggerOptions by merging defaults and options
 *   return pass-through middleware if logger is undefined
 *   exit early when predicate(getState, action) is false
 *   push logEntry into logBuffer
 *   record started, startedTime, prevState, action
 *   call next(action), optionally capturing errors
 *   record took and nextState
 *   resolve diff/diffPredicate
 *   printBuffer(logBuffer, optionsWithDiff)
 *   clear logBuffer
 *   rethrow captured error
 *   return next(action)'s return value
 */
template <typename State>
Middleware<State>
createLogger(const ReduxLoggerOptions<State> &Options =
                 ReduxLoggerOptions<State>()) {
  const ReduxLoggerOptions<State> LoggerOptions = Options;
  return LoggerOptions.bLoggerAvailable
             ? detail::activeLoggerMiddleware(LoggerOptions)
             : detail::passThroughMiddleware<State>();
}

// default export in redux-logger v3 is the logger with default settings.
template <typename State> Middleware<State> defaultLogger() {
  return createLogger<State>();
}

} // namespace logger
} // namespace rtk

#endif // REDUX_LOGGER_HPP
