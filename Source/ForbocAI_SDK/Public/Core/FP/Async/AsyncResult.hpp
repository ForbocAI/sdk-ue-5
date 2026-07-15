#pragma once

#include "Core/FP/Error/Error.hpp"

namespace func {
/**
 * @brief 16. AsyncResult (Functional Async Result Handling) A type for handling async operations that can succeed or fail, with support for chaining and error handling. Safe for async callbacks via shared state. Usage: auto result = AsyncResult<int>::create([]( std::function<void(int)> resolve, std::function<void(std::string)> reject) { // async operation }); result.then([](int value) { // success }).catch_([](std::string error) { // failure }).execute();
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature namespace detail
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct AsyncResult;

template <typename T>
AsyncResult<T>
createAsyncResult(std::function<void(std::function<void(T)>,
                                     std::function<void(std::string)>)>
                      executor);

template <typename T, typename Handler>
const AsyncResult<T> &thenAsync(const AsyncResult<T> &result,
                                Handler handler);

template <typename T, typename Handler>
const AsyncResult<T> &catchAsync(const AsyncResult<T> &result,
                                 Handler handler);

template <typename T> void executeAsync(const AsyncResult<T> &result);

inline AsyncResult<void>
createAsyncResult(std::function<void(std::function<void()>,
                                     std::function<void(std::string)>)>
                      executor);

template <typename Handler>
inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result,
                                          Handler handler);

template <typename Handler>
inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result,
                                           Handler handler);

inline void executeAsync(const AsyncResult<void> &result);

template <typename T> struct AsyncResult {
  struct State {
    std::function<void(std::function<void(T)>,
                       std::function<void(std::string)>)>
        executor;
    std::vector<std::function<void(T)>> successHandlers;
    std::vector<std::function<void(std::string)>> errorHandlers;
  };
  std::shared_ptr<State> state = std::make_shared<State>();

/**
 * @brief Builds an async result from an executor callback.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature static AsyncResult<T> create(std::function<void(std::function<void(T)>, std::function<void(std::string)>)> executor)
 *
 * User Story: As async composition code, I need a factory that captures an
   * executor so asynchronous work can be chained through one result type.
 */
  static AsyncResult<T>
  create(std::function<void(std::function<void(T)>,
                            std::function<void(std::string)>)>
             executor) {
    return createAsyncResult<T>(std::move(executor));
  }

/**
 * @brief Registers a success handler on the async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature const AsyncResult<T> &then(std::function<void(T)> handler) const
 *
 * User Story: As async composition code, I need success callbacks so resolved
   * values can trigger follow-up behavior without blocking.
 */
  const AsyncResult<T> &then(std::function<void(T)> handler) const {
    return thenAsync(*this, std::move(handler));
  }

/**
 * @brief Registers an error handler on the async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature const AsyncResult<T> &catch_(std::function<void(std::string)> handler) const
 *
 * User Story: As async composition code, I need error callbacks so rejected
   * work can surface failures through the same fluent API.
 */
  const AsyncResult<T> &catch_(std::function<void(std::string)> handler) const {
    return catchAsync(*this, std::move(handler));
  }

/**
 * @brief Executes the stored async operation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature void execute() const
 *
 * User Story: As async composition code, I need an explicit execute step so
   * async pipelines run only when the caller is ready to trigger them.
 */
  void execute() const { executeAsync(*this); }
};

} // namespace func
