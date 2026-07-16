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
 * @fn static AsyncResult<void> create(std::function<void(std::function<void()>, std::function<void(std::string)>)> executor)
 * @brief Builds a void async result from an executor callback.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
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
 * @fn const AsyncResult<void> &then(std::function<void()> handler) const
 * @brief Registers a success handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As async composition code, I need success callbacks so
 * completion-only tasks can notify later stages without return values.
 */
  const AsyncResult<void> &then(std::function<void()> handler) const {
    return thenAsync(*this, std::move(handler));
  }

/**
 * @fn const AsyncResult<void> & catch_(std::function<void(std::string)> handler) const
 * @brief Registers an error handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As async composition code, I need error callbacks so void
 * tasks can surface failures through the same fluent interface.
 */
  const AsyncResult<void> &
  catch_(std::function<void(std::string)> handler) const {
    return catchAsync(*this, std::move(handler));
  }

/**
 * @fn void execute() const
 * @brief Executes the stored void async operation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As async composition code, I need an explicit execute step so
 * completion-only async pipelines run on demand.
 */
  void execute() const { executeAsync(*this); }
};

namespace detail {
/** User Story: As a core fp async consumer, I need to invoke invoke success handlers recursive through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T> void invokeSuccessHandlersRecursive( const std::vector<std::function<void(T)>> &Handlers, size_t Index, T Value) */
template <typename T>
void invokeSuccessHandlersRecursive(
    const std::vector<std::function<void(T)>> &Handlers, size_t Index,
    T Value) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Value),
         invokeSuccessHandlersRecursive<T>(Handlers, Index + 1, Value));
}

/** User Story: As a core fp async consumer, I need to invoke invoke void handlers recursive through a stable signature so the core fp async workflow remains explicit and composable. @fn inline void invokeVoidHandlersRecursive(const std::vector<std::function<void()>> &Handlers, size_t Index) */
inline void
invokeVoidHandlersRecursive(const std::vector<std::function<void()>> &Handlers,
                            size_t Index) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](), invokeVoidHandlersRecursive(Handlers, Index + 1));
}

/** User Story: As a core fp async consumer, I need to invoke invoke error handlers recursive through a stable signature so the core fp async workflow remains explicit and composable. @fn inline void invokeErrorHandlersRecursive( const std::vector<std::function<void(std::string)>> &Handlers, size_t Index, const std::string &Error) */
inline void invokeErrorHandlersRecursive(
    const std::vector<std::function<void(std::string)>> &Handlers,
    size_t Index, const std::string &Error) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Error),
         invokeErrorHandlersRecursive(Handlers, Index + 1, Error));
}

/** User Story: As a core fp async consumer, I need to invoke run async executor through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T> void runAsyncExecutor( const std::shared_ptr<typename AsyncResult<T>::State> &CapturedState) */
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

/** User Story: As a core fp async consumer, I need to invoke run async executor through a stable signature so the core fp async workflow remains explicit and composable. @fn inline void runAsyncExecutor( const std::shared_ptr<typename AsyncResult<void>::State> &CapturedState) */
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

/** User Story: As a core fp async consumer, I need to invoke create async result through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T> AsyncResult<T> createAsyncResult(std::function<void(std::function<void(T)>, std::function<void(std::string)>)> executor) */
template <typename T>
AsyncResult<T>
createAsyncResult(std::function<void(std::function<void(T)>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<T> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

/** User Story: As a core fp async consumer, I need to invoke then async through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T, typename Handler> const AsyncResult<T> &thenAsync(const AsyncResult<T> &result, Handler handler) */
template <typename T, typename Handler>
const AsyncResult<T> &thenAsync(const AsyncResult<T> &result,
                                Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void(T)>(std::move(handler))),
                result)
             : result;
}

/** User Story: As a core fp async consumer, I need to invoke catch async through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T, typename Handler> const AsyncResult<T> &catchAsync(const AsyncResult<T> &result, Handler handler) */
template <typename T, typename Handler>
const AsyncResult<T> &catchAsync(const AsyncResult<T> &result,
                                 Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

/** User Story: As a core fp async consumer, I need to invoke execute async through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename T> void executeAsync(const AsyncResult<T> &result) */
template <typename T> void executeAsync(const AsyncResult<T> &result) {
  const std::shared_ptr<typename AsyncResult<T>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor<T>(CapturedState)
      : void();
}

/** User Story: As a core fp async consumer, I need to invoke create async result through a stable signature so the core fp async workflow remains explicit and composable. @fn inline AsyncResult<void> createAsyncResult(std::function<void(std::function<void()>, std::function<void(std::string)>)> executor) */
inline AsyncResult<void>
createAsyncResult(std::function<void(std::function<void()>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<void> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

/** User Story: As a core fp async consumer, I need to invoke then async through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename Handler> inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result, Handler handler) */
template <typename Handler>
inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result,
                                          Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void()>(std::move(handler))),
                result)
             : result;
}

/** User Story: As a core fp async consumer, I need to invoke catch async through a stable signature so the core fp async workflow remains explicit and composable. @fn template <typename Handler> inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result, Handler handler) */
template <typename Handler>
inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result,
                                           Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

/** User Story: As a core fp async consumer, I need to invoke execute async through a stable signature so the core fp async workflow remains explicit and composable. @fn inline void executeAsync(const AsyncResult<void> &result) */
inline void executeAsync(const AsyncResult<void> &result) {
  const std::shared_ptr<typename AsyncResult<void>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor(CapturedState)
      : void();
}

} // namespace func
