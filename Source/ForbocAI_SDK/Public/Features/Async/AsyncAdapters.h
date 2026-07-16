#pragma once

#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "HttpManager.h"
#include "HttpModule.h"
#include <stdexcept>

namespace AsyncAdapters {

/** User Story: As a features async consumer, I need to invoke wait for result through a stable signature so the features async workflow remains explicit and composable. @fn template <typename T> T waitForResult(func::AsyncResult<T> &&Async, double TimeoutSeconds = 15.0) */
template <typename T>
T waitForResult(func::AsyncResult<T> &&Async, double TimeoutSeconds = 15.0) {
  bool bCompleted = false;
  T Result{};
  FString Error;

  Async.then([&bCompleted, &Result](T Value) {
    Result = Value;
    bCompleted = true;
  }).catch_([&bCompleted, &Error](std::string Message) {
    Error = UTF8_TO_TCHAR(Message.c_str());
    bCompleted = true;
  });
  Async.execute();

  const double StartTime = FPlatformTime::Seconds();
  struct PollLoop {
    static void apply(bool &Completed, double Start, double Timeout) {
      (!Completed && (FPlatformTime::Seconds() - Start) < Timeout)
          ? (FTaskGraphInterface::Get().ProcessThreadUntilIdle(
                 ENamedThreads::GameThread),
             FHttpModule::Get().GetHttpManager().Tick(0.05f),
             FPlatformProcess::Sleep(0.05f),
             apply(Completed, Start, Timeout), void())
          : void();
    }
  };
  PollLoop::apply(bCompleted, StartTime, TimeoutSeconds);

  !Error.IsEmpty() ? throw std::runtime_error(TCHAR_TO_UTF8(*Error)) : (void)0;
  !bCompleted ? throw std::runtime_error("Timed out waiting for async result")
              : (void)0;
  return Result;
}

} // namespace AsyncAdapters
