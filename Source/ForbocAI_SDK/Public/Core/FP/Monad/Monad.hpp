#pragma once

#include "Core/FP/Functor/Functor.hpp"

namespace func {
/**
 * @brief Chains a Maybe-producing function onto a Maybe value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T, typename Func> auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value))
 *
 * User Story: As optional workflows, I need bind semantics so dependent Maybe
 * operations can short-circuit naturally on missing values.
 */
template <typename T, typename Func>
auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return m.hasValue ? f(m.value) : decltype(f(m.value)){false, {}};
}

template <typename T, typename Func>
auto maybe_map(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> {
  return fmap(m, f);
}

template <typename T, typename Func>
auto maybe_chain(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return mbind(m, f);
}

/**
 * @brief Chains an Either-producing function onto an Either success value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename E, typename T, typename Func> auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 *
 * User Story: As result workflows, I need bind semantics so failure branches
 * stop the pipeline while successes continue.
 */
template <typename E, typename T, typename Func>
auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return e.isLeft ? decltype(f(e.right)){true, e.left, {}} : f(e.right);
}

/**
 * @brief Maps the success branch of an Either and preserves failures.
 *
 * @signature template <typename E, typename T, typename Func> auto either_map(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))>
 *
 * User Story: As a result pipeline author, I need to transform successful
 * payloads without unpacking or disturbing the error branch.
 */
template <typename E, typename T, typename Func>
auto either_map(const Either<E, T> &e, Func f)
    -> Either<E, decltype(f(e.right))> {
  return fmap(e, f);
}

template <typename E, typename T, typename Func>
auto efmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))> {
  return fmap(e, f);
}

/**
 * @brief Chains an Either-producing function from a successful Either.
 *
 * @signature template <typename E, typename T, typename Func> auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 *
 * User Story: As a result pipeline author, I need to sequence fallible steps
 * so the first Left skips all downstream work.
 */
template <typename E, typename T, typename Func>
auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return ebind(e, f);
}

/**
 * @brief Maps the failure branch of an Either and preserves successes.
 *
 * @signature template <typename E, typename T, typename Func> auto either_map_left(const Either<E, T> &e, Func f) -> Either<decltype(f(e.left)), T>
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
 * @brief Extracts a Maybe value or returns the provided default.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> T or_else(const Maybe<T> &m, const T &def)
 *
 * User Story: As boundary code, I need a defaulting helper so Maybe values can
 * be converted into concrete values at integration points.
 */
template <typename T> T or_else(const Maybe<T> &m, const T &def) {
  return m.hasValue ? m.value : def;
}

template <typename T> T orElse(const Maybe<T> &m, const T &def) {
  return or_else(m, def);
}

template <typename T, typename DefaultFactory>
T maybe_or_else(const Maybe<T> &m, DefaultFactory defaultFactory) {
  return m.hasValue ? m.value : defaultFactory();
}

/**
 * @brief Extracts an Either success value or returns a default for failures.
 *
 * @signature template <typename E, typename T> T either_or_else(const Either<E, T> &e, const T &def)
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
 * @brief Pattern matches on a Maybe with Just and Nothing callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T, typename FJust, typename FNothing> auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing) -> decltype(onJust(m.value))
 *
 * User Story: As boundary code, I need pattern matching on Maybe so success and
 * empty branches can be handled declaratively.
 */
template <typename T, typename FJust, typename FNothing>
auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return m.hasValue ? onJust(m.value) : onNothing();
}

template <typename T, typename FJust, typename FNothing>
auto maybe_match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return match(m, onJust, onNothing);
}

/**
 * @brief Pattern matches on an Either with error and success callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename E, typename T, typename FLeft, typename FRight> auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight) -> decltype(onRight(e.right))
 *
 * User Story: As boundary code, I need pattern matching on Either so success
 * and failure handling stay explicit and type-safe.
 */
template <typename E, typename T, typename FLeft, typename FRight>
auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return e.isLeft ? onLeft(e.left) : onRight(e.right);
}

template <typename E, typename T, typename FLeft, typename FRight>
auto either_match(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return ematch(e, onLeft, onRight);
}

} // namespace func
