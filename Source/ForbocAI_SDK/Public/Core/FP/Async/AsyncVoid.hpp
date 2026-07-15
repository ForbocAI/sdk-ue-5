#pragma once

#include "Core/FP/Async/AsyncResult.hpp"

namespace func {
/**
 * @brief Specialization for void
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <> struct AsyncResult<void>
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <> struct AsyncResult<void> {
  struct State {
    std::function<void(std::function<void()>, std::function<void(std::string)>)>
        executor;
    std::vector<std::function<void()>> successHandlers;
    std::vector<std::function<void(std::string)>> errorHandlers;
  };
  std::shared_ptr<State> state = std::make_shared<State>();

/**
 * @brief Builds a void async result from an executor callback.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature static AsyncResult<void> create(std::function<void(std::function<void()>, std::function<void(std::string)>)> executor)
 *
 * User Story: As async composition code, I need a void factory so fire-and-
   * signal tasks can share the same chaining surface as valued tasks.
 */
  static AsyncResult<void>
  create(std::function<void(std::function<void()>,
                            std::function<void(std::string)>)>
             executor) {
    return createAsyncResult(std::move(executor));
  }

/**
 * @brief Registers a success handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature const AsyncResult<void> &then(std::function<void()> handler) const
 *
 * User Story: As async composition code, I need success callbacks so
   * completion-only tasks can notify later stages without return values.
 */
  const AsyncResult<void> &then(std::function<void()> handler) const {
    return thenAsync(*this, std::move(handler));
  }

/**
 * @brief Registers an error handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature const AsyncResult<void> & catch_(std::function<void(std::string)> handler) const
 *
 * User Story: As async composition code, I need error callbacks so void
   * tasks can surface failures through the same fluent interface.
 */
  const AsyncResult<void> &
  catch_(std::function<void(std::string)> handler) const {
    return catchAsync(*this, std::move(handler));
  }

/**
 * @brief Executes the stored void async operation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature void execute() const
 *
 * User Story: As async composition code, I need an explicit execute step so
   * completion-only async pipelines run on demand.
 */
  void execute() const { executeAsync(*this); }
};

namespace detail {
template <typename T>
void invokeSuccessHandlersRecursive(
    const std::vector<std::function<void(T)>> &Handlers, size_t Index,
    T Value) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Value),
         invokeSuccessHandlersRecursive<T>(Handlers, Index + 1, Value));
}

inline void
invokeVoidHandlersRecursive(const std::vector<std::function<void()>> &Handlers,
                            size_t Index) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](), invokeVoidHandlersRecursive(Handlers, Index + 1));
}

inline void invokeErrorHandlersRecursive(
    const std::vector<std::function<void(std::string)>> &Handlers,
    size_t Index, const std::string &Error) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Error),
         invokeErrorHandlersRecursive(Handlers, Index + 1, Error));
}

template <typename T>
void runAsyncExecutor(
    const std::shared_ptr<typename AsyncResult<T>::State> &CapturedState) {
  CapturedState->executor(
      [CapturedState](T Value) {
        invokeSuccessHandlersRecursive<T>(CapturedState->successHandlers, 0,
                                          Value);
      },
      [CapturedState](std::string Error) {
        invokeErrorHandlersRecursive(CapturedState->errorHandlers, 0, Error);
      });
}

inline void runAsyncExecutor(
    const std::shared_ptr<typename AsyncResult<void>::State> &CapturedState) {
  CapturedState->executor(
      [CapturedState]() {
        invokeVoidHandlersRecursive(CapturedState->successHandlers, 0);
      },
      [CapturedState](std::string Error) {
        invokeErrorHandlersRecursive(CapturedState->errorHandlers, 0, Error);
      });
}
} // namespace detail

template <typename T>
AsyncResult<T>
createAsyncResult(std::function<void(std::function<void(T)>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<T> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

template <typename T, typename Handler>
const AsyncResult<T> &thenAsync(const AsyncResult<T> &result,
                                Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void(T)>(std::move(handler))),
                result)
             : result;
}

template <typename T, typename Handler>
const AsyncResult<T> &catchAsync(const AsyncResult<T> &result,
                                 Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

template <typename T> void executeAsync(const AsyncResult<T> &result) {
  const std::shared_ptr<typename AsyncResult<T>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor<T>(CapturedState)
      : void();
}

inline AsyncResult<void>
createAsyncResult(std::function<void(std::function<void()>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<void> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

template <typename Handler>
inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result,
                                          Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void()>(std::move(handler))),
                result)
             : result;
}

template <typename Handler>
inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result,
                                           Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

inline void executeAsync(const AsyncResult<void> &result) {
  const std::shared_ptr<typename AsyncResult<void>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor(CapturedState)
      : void();
}

} // namespace func
