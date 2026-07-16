#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 6. DATA: Lazy (Deferred Evaluation) Memoized deferred computation. The thunk is evaluated at most once on first access via eval(). Construction: use the lazy() factory function. Access:       use the eval() free function. Note: Not thread-safe. Intended for single-thread use (e.g. game thread).
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> struct Lazy
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct Lazy {
  std::function<T()> thunk;
  mutable std::shared_ptr<T> cached;
};

/**
 * @fn template <typename F> auto lazy(F &&f) -> Lazy<decltype(f())>
 * @brief Wraps a thunk so it is evaluated once on first access.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As deferred setup code, I need lazy values so expensive work only
 * runs when a caller actually needs the result.
 */
template <typename F> auto lazy(F &&f) -> Lazy<decltype(f())> {
  return Lazy<decltype(f())>{std::forward<F>(f), nullptr};
}

namespace detail {
/** User Story: As a core fp lazy consumer, I need to invoke cache lazy value through a stable signature so the core fp lazy workflow remains explicit and composable. @fn template <typename T> const T &cacheLazyValue(const Lazy<T> &lz) */
template <typename T> const T &cacheLazyValue(const Lazy<T> &lz) {
  lz.cached = std::make_shared<T>(lz.thunk());
  return *lz.cached;
}
} // namespace detail

/**
 * @fn template <typename T> const T &eval(const Lazy<T> &lz)
 * @brief Forces a lazy value and memoizes the computed result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As deferred setup code, I need an explicit force helper so lazy
 * values can be materialized through one consistent API.
 */
template <typename T> const T &eval(const Lazy<T> &lz) {
  return lz.cached ? *lz.cached : detail::cacheLazyValue(lz);
}

} // namespace func
