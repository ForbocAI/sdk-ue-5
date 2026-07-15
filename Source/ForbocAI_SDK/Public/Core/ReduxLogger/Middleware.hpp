#pragma once

#include "Core/ReduxLogger/Formatting.hpp"

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace rtk::logger::detail {

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
  } catch (const std::exception &Exception) {
    return loggedFailure(Options.ErrorTransformer(
        FString(UTF8_TO_TCHAR(Exception.what()))));
  } catch (...) {
    return loggedFailure(Options.ErrorTransformer(TEXT("<unknown error>")));
  }
#endif
}

template <typename State>
LoggedDispatchResult dispatchNext(const ReduxLoggerOptions<State> &Options,
                                  Dispatcher Next,
                                  const AnyAction &Action) {
  return Options.bLogErrors
             ? dispatchWithErrorCapture(Options, Next, Action)
             : loggedSuccess(Next(Action));
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
                 const std::function<const State &()> &GetState,
                 const AnyAction &Action) {
  ReduxLoggerOptions<State> PrintOptions = Options;
  PrintOptions.bDiff = Options.bDiff && Options.DiffPredicate
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
    const MiddlewareApi<State> &Api, Dispatcher Next, const AnyAction &Action,
    const std::function<const State &()> &GetState) {
  const LogEntry StartedEntry = startLogEntry(Options, Api, Action);
  const LoggedDispatchResult DispatchResult =
      dispatchNext(Options, Next, Action);
  const LogEntry Entry = finishLogEntry(
      Options, Api, applyDispatchResult(StartedEntry, DispatchResult));
  LogBuffer->push_back(Entry);
  printBuffer(*LogBuffer, withResolvedDiff(Options, GetState, Action), Logger);
  LogBuffer->clear();
  rethrowLoggedError(Entry);
  return DispatchResult.ReturnedValue;
}

template <typename State>
AnyAction dispatchOrSkip(
    const ReduxLoggerOptions<State> &Options,
    const std::function<void(const FString &)> &Logger,
    const std::shared_ptr<std::vector<LogEntry>> &LogBuffer,
    const MiddlewareApi<State> &Api, Dispatcher Next, const AnyAction &Action) {
  const std::function<const State &()> GetState = [Api]() -> const State & {
    return Api.getState();
  };
  return Options.Predicate && !Options.Predicate(GetState, Action)
             ? Next(Action)
             : dispatchWithLogging(Options, Logger, LogBuffer, Api, Next,
                                   Action, GetState);
}

template <typename State> Middleware<State> passThroughMiddleware() {
  return [](const MiddlewareApi<State> &)
             -> std::function<Dispatcher(Dispatcher)> {
    return [](Dispatcher Next) -> Dispatcher {
      return [Next](const AnyAction &Action) { return Next(Action); };
    };
  };
}

template <typename State>
std::function<void(const FString &)>
resolveLogger(const ReduxLoggerOptions<State> &Options) {
  return Options.Logger ? Options.Logger
                        : [](const FString &Message) {
                            UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
                          };
}

template <typename State>
Middleware<State>
activeLoggerMiddleware(const ReduxLoggerOptions<State> &Options) {
  const std::function<void(const FString &)> Logger = resolveLogger(Options);
  const auto LogBuffer = std::make_shared<std::vector<LogEntry>>();
  return [Options, Logger, LogBuffer](const MiddlewareApi<State> &Api)
             -> std::function<Dispatcher(Dispatcher)> {
    return [Options, Logger, LogBuffer, Api](Dispatcher Next) -> Dispatcher {
      return [Options, Logger, LogBuffer, Api,
              Next](const AnyAction &Action) {
        return dispatchOrSkip(Options, Logger, LogBuffer, Api, Next, Action);
      };
    };
  };
}

} // namespace rtk::logger::detail

namespace rtk::logger {

template <typename State>
Middleware<State>
createLogger(const ReduxLoggerOptions<State> &Options =
                 ReduxLoggerOptions<State>()) {
  return Options.bLoggerAvailable
             ? detail::activeLoggerMiddleware(Options)
             : detail::passThroughMiddleware<State>();
}

template <typename State> Middleware<State> defaultLogger() {
  return createLogger<State>();
}

} // namespace rtk::logger
