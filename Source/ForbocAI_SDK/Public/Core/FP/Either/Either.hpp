#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 4. DATA: Either (Result/Error Monad) A sum type for computations that can fail. Convention: Left = error, Right = success. Pure data struct — no methods. Construction: use factory functions make_left() / make_right() Operations:   use free functions fmap() / ebind() Requires: E and T are default-constructible.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename E, typename T> struct Either
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename E, typename T> struct Either {
  bool isLeft;
  E left;
  T right;
};

/**
 * @fn template <typename E, typename T> Either<E, T> make_left(E e)
 * @brief Constructs the error branch of an Either value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result-returning code, I need a clear error constructor so
 * failure paths remain explicit in functional chains.
 */
template <typename E, typename T> Either<E, T> make_left(E e) {
  return Either<E, T>{true, std::move(e), T{}};
}

/**
 * @fn template <typename E, typename T> Either<E, T> make_left(E e, T dummy)
 * @brief Constructs the error branch while preserving an explicit default payload.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result-returning code, I need an error constructor that also
 * satisfies payload shape requirements in C++11.
 */
template <typename E, typename T> Either<E, T> make_left(E e, T dummy) {
  return Either<E, T>{true, std::move(e), std::move(dummy)};
}

/**
 * @fn template <typename E, typename T> Either<E, T> make_right(T v)
 * @brief Constructs the success branch of an Either value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result-returning code, I need a clear success constructor so
 * successful values move through pipelines predictably.
 */
template <typename E, typename T> Either<E, T> make_right(T v) {
  return Either<E, T>{false, E{}, std::move(v)};
}

/**
 * @fn template <typename E, typename T> Either<E, T> make_right(E dummy, T v)
 * @brief Constructs the success branch while preserving an explicit default error value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result-returning code, I need a success constructor that also
 * preserves error shape requirements in C++11.
 */
template <typename E, typename T> Either<E, T> make_right(E dummy, T v) {
  return Either<E, T>{false, std::move(dummy), std::move(v)};
}

/**
 * @fn template <typename E, typename T> Either<E, T> left(E e)
 * @brief Alias for constructing the error branch of an Either.
 *
 *
 * User Story: As a functional-programming caller, I need the common Left name
 * so failure construction matches Rust, TypeScript, and GDScript examples.
 */
template <typename E, typename T> Either<E, T> left(E e) {
  return make_left<E, T>(std::move(e));
}

/**
 * @fn template <typename E, typename T> Either<E, T> right(T v)
 * @brief Alias for constructing the success branch of an Either.
 *
 *
 * User Story: As a functional-programming caller, I need the common Right name
 * so successful result construction is portable across SDK cores.
 */
template <typename E, typename T> Either<E, T> right(T v) {
  return make_right<E, T>(std::move(v));
}

/** User Story: As a core fp either consumer, I need to invoke is left through a stable signature so the core fp either workflow remains explicit and composable. @fn template <typename E, typename T> bool isLeft(const Either<E, T> &e) */
template <typename E, typename T> bool isLeft(const Either<E, T> &e) {
  return e.isLeft;
}

/** User Story: As a core fp either consumer, I need to invoke is right through a stable signature so the core fp either workflow remains explicit and composable. @fn template <typename E, typename T> bool isRight(const Either<E, T> &e) */
template <typename E, typename T> bool isRight(const Either<E, T> &e) {
  return !e.isLeft;
}

} // namespace func
