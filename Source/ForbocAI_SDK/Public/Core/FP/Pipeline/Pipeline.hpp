#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 8. DATA: Pipeline (Value Transformation) Fluent chain for threading a value through a series of pure transformations using operator|. Construction: use the pipe() factory function. Chaining:     use operator| with transform functions. Extraction:   access the .val member directly. Usage: auto add1 = [](int x) { return x + 1; }; auto mul2 = [](int x) { return x * 2; }; auto result = func::pipe(5) | add1 | mul2; int final = result.val;  // 12
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> struct Pipeline
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct Pipeline {
  T val;
};

/**
 * @fn template <typename T> Pipeline<T> pipe(T v)
 * @brief Starts a pipeline with an initial value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional composition code, I need a pipeline entry point so
 * value-threading reads clearly in C++11 call sites.
 */
template <typename T> Pipeline<T> pipe(T v) {
  return Pipeline<T>{std::move(v)};
}

template <typename F> struct Tapped {
  F f;

  /** User Story: As a core fp pipeline consumer, I need to invoke the callable value through a stable signature so the core fp pipeline workflow remains explicit and composable. @fn template <typename T> T operator()(T value) */
  template <typename T> T operator()(T value) {
    f(value);
    return value;
  }
};

/** User Story: As a core fp pipeline consumer, I need to invoke tap through a stable signature so the core fp pipeline workflow remains explicit and composable. @fn template <typename F> Tapped<F> tap(F f) */
template <typename F> Tapped<F> tap(F f) { return Tapped<F>{std::move(f)}; }

template <typename F> struct TapMut {
  F f;

  /** User Story: As a core fp pipeline consumer, I need to invoke the callable value through a stable signature so the core fp pipeline workflow remains explicit and composable. @fn template <typename T> T &operator()(T &value) */
  template <typename T> T &operator()(T &value) {
    f(value);
    return value;
  }
};

/** User Story: As a core fp pipeline consumer, I need to invoke tap mut through a stable signature so the core fp pipeline workflow remains explicit and composable. @fn template <typename F> TapMut<F> tap_mut(F f) */
template <typename F> TapMut<F> tap_mut(F f) {
  return TapMut<F>{std::move(f)};
}

/**
 * @fn template <typename T, typename F> auto operator|(const Pipeline<T> &p, F f) -> Pipeline<decltype(f(p.val))>
 * @brief operator| for chaining lvalue-backed pipelines.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename T, typename F>
auto operator|(const Pipeline<T> &p, F f) -> Pipeline<decltype(f(p.val))> {
  return Pipeline<decltype(f(p.val))>{f(p.val)};
}

/**
 * @fn template <typename T, typename F> auto operator|(Pipeline<T> &&p, F f) -> Pipeline<decltype(f(std::move(p.val)))>
 * @brief operator| for chaining move-only or ownership-transferring values.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename T, typename F>
auto operator|(Pipeline<T> &&p, F f)
    -> Pipeline<decltype(f(std::move(p.val)))> {
  return Pipeline<decltype(f(std::move(p.val)))>{f(std::move(p.val))};
}

} // namespace func
