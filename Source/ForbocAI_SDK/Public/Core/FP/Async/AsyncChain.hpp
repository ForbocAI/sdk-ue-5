#pragma once

#include "Core/FP/Async/AsyncVoid.hpp"

namespace func {
/**
 * @brief 18. AsyncChain (Helpers for chaining AsyncResults)
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature namespace AsyncChain
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

namespace AsyncChain {
/**
 * @fn template <typename T, typename U, typename F> auto then(const AsyncResult<T> &res, F f) -> AsyncResult<U>
 * @brief Chains one AsyncResult into another async-producing transformation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As async thunk composition, I need async chaining so one async
 * result can feed into the next without nested callback plumbing.
 */
template <typename T, typename U, typename F>
auto then(const AsyncResult<T> &res, F f) -> AsyncResult<U> {
  return createAsyncResult<U>(
      [res, f](std::function<void(U)> resolve,
               std::function<void(std::string)> reject) {
        AsyncResult<T> Source = res;
        thenAsync(Source, [f, resolve, reject](T val) {
          AsyncResult<U> Next = f(val);
          thenAsync(Next, resolve);
          catchAsync(Next, reject);
          executeAsync(Next);
        });
        catchAsync(Source, reject);
        executeAsync(Source);
      });
}
} // namespace AsyncChain

} // namespace func
