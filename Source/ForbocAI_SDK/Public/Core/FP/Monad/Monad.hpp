#pragma once

#include "Core/FP/Functor/Functor.hpp"

namespace func {
/**
 * @fn template <typename T, typename Func> auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value))
 * @brief Chains a Maybe-producing function onto a Maybe value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As optional workflows, I need bind semantics so dependent Maybe
 * operations can short-circuit naturally on missing values.
 */
template <typename T, typename Func>
auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return m.hasValue ? f(m.value) : decltype(f(m.value)){false, {}};
}

/** User Story: As a core fp monad consumer, I need to invoke maybe map through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename T, typename Func> auto maybe_map(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> */
template <typename T, typename Func>
auto maybe_map(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> {
  return fmap(m, f);
}

/** User Story: As a core fp monad consumer, I need to invoke maybe chain through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename T, typename Func> auto maybe_chain(const Maybe<T> &m, Func f) -> decltype(f(m.value)) */
template <typename T, typename Func>
auto maybe_chain(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return mbind(m, f);
}

/**
 * @fn template <typename E, typename T, typename Func> auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 * @brief Chains an Either-producing function onto an Either success value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result workflows, I need bind semantics so failure branches
 * stop the pipeline while successes continue.
 */
template <typename E, typename T, typename Func>
auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return e.isLeft ? decltype(f(e.right)){true, e.left, {}} : f(e.right);
}

/**
 * @fn template <typename E, typename T, typename Func> auto either_map(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))>
 * @brief Maps the success branch of an Either and preserves failures.
 *
 *
 * User Story: As a result pipeline author, I need to transform successful
 * payloads without unpacking or disturbing the error branch.
 */
template <typename E, typename T, typename Func>
auto either_map(const Either<E, T> &e, Func f)
    -> Either<E, decltype(f(e.right))> {
  return fmap(e, f);
}

/** User Story: As a core fp monad consumer, I need to invoke efmap through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename E, typename T, typename Func> auto efmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))> */
template <typename E, typename T, typename Func>
auto efmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))> {
  return fmap(e, f);
}

/**
 * @fn template <typename E, typename T, typename Func> auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 * @brief Chains an Either-producing function from a successful Either.
 *
 *
 * User Story: As a result pipeline author, I need to sequence fallible steps
 * so the first Left skips all downstream work.
 */
template <typename E, typename T, typename Func>
auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return ebind(e, f);
}

/**
 * @fn template <typename E, typename T, typename Func> auto either_map_left(const Either<E, T> &e, Func f) -> Either<decltype(f(e.left)), T>
 * @brief Maps the failure branch of an Either and preserves successes.
 *
 *
 * User Story: As a boundary author, I need to translate low-level errors into
 * domain errors without touching the successful payload path.
 */
template <typename E, typename T, typename Func>
auto either_map_left(const Either<E, T> &e, Func f)
    -> Either<decltype(f(e.left)), T> {
  typedef decltype(f(e.left)) NextError;
  std::function<Either<NextError, T>()> Cases[2] = {
      [&]() { return make_right<NextError, T>(e.right); },
      [&]() { return make_left<NextError, T>(f(e.left)); }};
  return Cases[static_cast<size_t>(e.isLeft)]();
}

/**
 * @fn template <typename T> T or_else(const Maybe<T> &m, const T &def)
 * @brief Extracts a Maybe value or returns the provided default.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As boundary code, I need a defaulting helper so Maybe values can
 * be converted into concrete values at integration points.
 */
template <typename T> T or_else(const Maybe<T> &m, const T &def) {
  return m.hasValue ? m.value : def;
}

/** User Story: As a core fp monad consumer, I need to invoke or else through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename T> T orElse(const Maybe<T> &m, const T &def) */
template <typename T> T orElse(const Maybe<T> &m, const T &def) {
  return or_else(m, def);
}

/** User Story: As a core fp monad consumer, I need to invoke maybe or else through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename T, typename DefaultFactory> T maybe_or_else(const Maybe<T> &m, DefaultFactory defaultFactory) */
template <typename T, typename DefaultFactory>
T maybe_or_else(const Maybe<T> &m, DefaultFactory defaultFactory) {
  return m.hasValue ? m.value : defaultFactory();
}

/**
 * @fn template <typename E, typename T> T either_or_else(const Either<E, T> &e, const T &def)
 * @brief Extracts an Either success value or returns a default for failures.
 *
 *
 * User Story: As an integration author, I need to collapse recoverable
 * failures into a concrete default at engine and UI boundaries.
 */
template <typename E, typename T>
T either_or_else(const Either<E, T> &e, const T &def) {
  std::function<T()> Cases[2] = {[&]() { return e.right; },
                                 [&]() { return def; }};
  return Cases[static_cast<size_t>(e.isLeft)]();
}

/**
 * @fn template <typename T, typename FJust, typename FNothing> auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing) -> decltype(onJust(m.value))
 * @brief Pattern matches on a Maybe with Just and Nothing callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As boundary code, I need pattern matching on Maybe so success and
 * empty branches can be handled declaratively.
 */
template <typename T, typename FJust, typename FNothing>
auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return m.hasValue ? onJust(m.value) : onNothing();
}

/** User Story: As a core fp monad consumer, I need to invoke maybe match through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename T, typename FJust, typename FNothing> auto maybe_match(const Maybe<T> &m, FJust onJust, FNothing onNothing) -> decltype(onJust(m.value)) */
template <typename T, typename FJust, typename FNothing>
auto maybe_match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return match(m, onJust, onNothing);
}

/**
 * @fn template <typename E, typename T, typename FLeft, typename FRight> auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight) -> decltype(onRight(e.right))
 * @brief Pattern matches on an Either with error and success callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As boundary code, I need pattern matching on Either so success
 * and failure handling stay explicit and type-safe.
 */
template <typename E, typename T, typename FLeft, typename FRight>
auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return e.isLeft ? onLeft(e.left) : onRight(e.right);
}

/** User Story: As a core fp monad consumer, I need to invoke either match through a stable signature so the core fp monad workflow remains explicit and composable. @fn template <typename E, typename T, typename FLeft, typename FRight> auto either_match(const Either<E, T> &e, FLeft onLeft, FRight onRight) -> decltype(onRight(e.right)) */
template <typename E, typename T, typename FLeft, typename FRight>
auto either_match(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return ematch(e, onLeft, onRight);
}

} // namespace func
