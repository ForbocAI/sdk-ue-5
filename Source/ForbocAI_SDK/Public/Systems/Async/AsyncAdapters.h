#pragma once

#include "Async/TaskGraphInterfaces.h"
#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Components/Async/AsyncTypes.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include <memory>
#include <mutex>
#include <stdexcept>

namespace AsyncAdapters {

/** User Story: As an Async adapter consumer, I need one immutable wait policy shared by every blocking adaptation without linking host modules to the SDK data loader. @fn FORBOCAI_SDK_API const FAsyncWaitSettings &asyncWaitSettings() */
FORBOCAI_SDK_API const FAsyncWaitSettings &asyncWaitSettings();

namespace detail {

template <typename T> struct TWaitState {
  std::mutex Mutex;
  bool bAccepting = true;
  bool bCompleted = false;
  T Result{};
  FString Error;
};

/** User Story: As a blocking async adapter, I need completion written into shared state so a late callback can never reference an expired stack frame. @fn template <typename T> void resolveWaitState(const std::shared_ptr<TWaitState<T>> &State, T Value) */
template <typename T>
void resolveWaitState(const std::shared_ptr<TWaitState<T>> &State, T Value) {
  const std::lock_guard<std::mutex> Lock(State->Mutex);
  State->bAccepting
      ? (State->Result = std::move(Value), State->bCompleted = true,
         State->bAccepting = false, void())
      : void();
}

/** User Story: As a blocking async adapter, I need rejection written into shared state so a late callback can never reference an expired stack frame. @fn template <typename T> void rejectWaitState(const std::shared_ptr<TWaitState<T>> &State, const std::string &Message) */
template <typename T>
void rejectWaitState(const std::shared_ptr<TWaitState<T>> &State,
                     const std::string &Message) {
  const std::lock_guard<std::mutex> Lock(State->Mutex);
  State->bAccepting
      ? (State->Error = UTF8_TO_TCHAR(Message.c_str()),
         State->bCompleted = true, State->bAccepting = false, void())
      : void();
}

/** User Story: As a blocking async adapter, I need one synchronized completion predicate so polling cannot race a transport callback. @fn template <typename T> bool isWaitComplete(const std::shared_ptr<TWaitState<T>> &State) */
template <typename T>
bool isWaitComplete(const std::shared_ptr<TWaitState<T>> &State) {
  const std::lock_guard<std::mutex> Lock(State->Mutex);
  return State->bCompleted;
}

/** User Story: As a blocking async adapter, I need an optional deadline predicate so normal RTK Query lifecycles settle without a competing timeout while bounded callers can still opt in. @fn inline bool isWaitWithinDeadline(double StartedAt, double TimeoutSeconds, double UnboundedTimeoutSeconds) */
inline bool isWaitWithinDeadline(double StartedAt, double TimeoutSeconds,
                                 double UnboundedTimeoutSeconds) {
  return TimeoutSeconds <= UnboundedTimeoutSeconds ||
         (FPlatformTime::Seconds() - StartedAt) < TimeoutSeconds;
}

/** User Story: As a blocking async adapter, I need rejection and timeout represented as data so engine boundaries can recover without an exception escaping. @fn template <typename T> func::Either<FString, T> readWaitState(const std::shared_ptr<TWaitState<T>> &State, const FString &TimeoutMessage) */
template <typename T>
func::Either<FString, T>
readWaitState(const std::shared_ptr<TWaitState<T>> &State,
              const FString &TimeoutMessage) {
  const std::lock_guard<std::mutex> Lock(State->Mutex);
  State->bAccepting = false;
  return !State->Error.IsEmpty()
             ? func::left<FString, T>(State->Error)
             : !State->bCompleted
                   ? func::left<FString, T>(TimeoutMessage)
                   : func::right<FString, T>(State->Result);
}

} // namespace detail

/** User Story: As an engine integration, I need an explicit async outcome so transport rejection remains recoverable data at the host boundary. @fn template <typename T> func::Either<FString, T> waitForEither(func::AsyncResult<T> &&Async, double TimeoutSeconds) */
template <typename T>
func::Either<FString, T> waitForEither(func::AsyncResult<T> &&Async,
                                       double TimeoutSeconds) {
  const FAsyncWaitSettings &Settings = asyncWaitSettings();
  const std::shared_ptr<detail::TWaitState<T>> State =
      std::make_shared<detail::TWaitState<T>>();

  Async.then([State](T Value) {
    detail::resolveWaitState(State, std::move(Value));
  }).catch_([State](std::string Message) {
    detail::rejectWaitState(State, Message);
  });
  Async.execute();

  const double StartTime = FPlatformTime::Seconds();
  struct PollLoop {
    static void apply(const std::shared_ptr<detail::TWaitState<T>> &State,
                      double Start, double Timeout,
                      const FAsyncWaitSettings &Settings) {
      (!detail::isWaitComplete(State) &&
       detail::isWaitWithinDeadline(
           Start, Timeout, Settings.Timing.UnboundedTimeoutSeconds))
          ? (FTaskGraphInterface::Get().ProcessThreadUntilIdle(
                 ENamedThreads::GameThread),
             FHttpModule::Get().GetHttpManager().Tick(
                 Settings.Timing.PollIntervalSeconds),
             FPlatformProcess::Sleep(Settings.Timing.PollIntervalSeconds),
             apply(State, Start, Timeout, Settings), void())
          : void();
    }
  };
  PollLoop::apply(State, StartTime, TimeoutSeconds, Settings);

  return detail::readWaitState(State, Settings.Messages.Timeout);
}

/** User Story: As an engine integration, I need the RTK Query lifecycle represented as an explicit outcome without a competing outer deadline. @fn template <typename T> func::Either<FString, T> waitForEither(func::AsyncResult<T> &&Async) */
template <typename T>
func::Either<FString, T> waitForEither(func::AsyncResult<T> &&Async) {
  return waitForEither(
      std::move(Async), asyncWaitSettings().Timing.UnboundedTimeoutSeconds);
}

/** User Story: As a CLI async consumer, I need an explicit finite timeout available for bounded non-query operations while the CLI error boundary retains failure formatting. @fn template <typename T> T waitForResult(func::AsyncResult<T> &&Async, double TimeoutSeconds) */
template <typename T>
T waitForResult(func::AsyncResult<T> &&Async, double TimeoutSeconds) {
  return func::ematch(
      waitForEither(std::move(Async), TimeoutSeconds),
      [](const FString &Error) -> T {
        throw std::runtime_error(TCHAR_TO_UTF8(*Error));
      },
      [](const T &Value) { return Value; });
}

/** User Story: As a features async consumer, I need to wait for the RTK Query lifecycle without imposing a competing outer deadline. @fn template <typename T> T waitForResult(func::AsyncResult<T> &&Async) */
template <typename T>
T waitForResult(func::AsyncResult<T> &&Async) {
  return waitForResult(
      std::move(Async), asyncWaitSettings().Timing.UnboundedTimeoutSeconds);
}

} // namespace AsyncAdapters
