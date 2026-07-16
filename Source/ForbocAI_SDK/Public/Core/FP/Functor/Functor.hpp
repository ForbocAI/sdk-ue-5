#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "Core/FP/Either/Either.hpp"

namespace func {
/**
 * @fn template <typename A, typename B, typename Combine> auto lift2(const Maybe<A> &a, const Maybe<B> &b, Combine combine) -> Maybe<decltype(combine(a.value, b.value))>
 * @brief Combines two Maybe values when both are present.
 *
 *
 * User Story: As data assembly code, I need small optional records to compose
 * without nested matches or substitute values.
 */
template <typename A, typename B, typename Combine>
auto lift2(const Maybe<A> &a, const Maybe<B> &b, Combine combine)
    -> Maybe<decltype(combine(a.value, b.value))> {
  typedef decltype(combine(a.value, b.value)) R;
  return (a.hasValue && b.hasValue) ? just(combine(a.value, b.value))
                                    : nothing<R>();
}

/**
 * @fn template <typename A, typename B, typename C, typename Combine> auto lift3(const Maybe<A> &a, const Maybe<B> &b, const Maybe<C> &c, Combine combine) -> Maybe<decltype(combine(a.value, b.value, c.value))>
 * @brief Combines three Maybe values when all are present.
 *
 *
 * User Story: As JSON and ECS seed code, I need small required-field groups to
 * assemble through one reusable optional combinator.
 */
template <typename A, typename B, typename C, typename Combine>
auto lift3(const Maybe<A> &a, const Maybe<B> &b, const Maybe<C> &c,
           Combine combine) -> Maybe<decltype(combine(a.value, b.value, c.value))> {
  typedef decltype(combine(a.value, b.value, c.value)) R;
  return (a.hasValue && b.hasValue && c.hasValue)
             ? just(combine(a.value, b.value, c.value))
             : nothing<R>();
}


/**
 * @fn template <typename T, typename Func> auto fmap(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))>
 * @brief Maps a function across the populated branch of a Maybe.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As optional transformations, I need fmap on Maybe so values can
 * be transformed without unwrapping and rewrapping by hand.
 */
template <typename T, typename Func>
auto fmap(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> {
  typedef decltype(f(m.value)) U;
  return m.hasValue ? Maybe<U>{true, f(m.value)} : Maybe<U>{false, U{}};
}

/**
 * @fn template <typename E, typename T, typename Func> auto fmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))>
 * @brief Maps a function across the success branch of an Either.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As result transformations, I need fmap on Either so success
 * values can be transformed while preserving failures unchanged.
 */
template <typename E, typename T, typename Func>
auto fmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))> {
  typedef decltype(f(e.right)) U;
  return e.isLeft ? Either<E, U>{true, e.left, U{}}
                  : Either<E, U>{false, E{}, f(e.right)};
}

/**
 * @brief Maps a function across every element in a vector.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature namespace detail
 *
 * User Story: As collection transformations, I need fmap on vectors so
 * element-wise mapping follows the same functional style as Maybe and Either.
 */
namespace detail {
/** User Story: As a core fp functor consumer, I need to invoke fmap vector recursive through a stable signature so the core fp functor workflow remains explicit and composable. @fn template <typename T, typename Func, typename U> std::vector<U> fmapVectorRecursive(const std::vector<T> &vec, Func f, size_t index, std::vector<U> result) */
template <typename T, typename Func, typename U>
std::vector<U> fmapVectorRecursive(const std::vector<T> &vec, Func f,
                                   size_t index, std::vector<U> result) {
  return index == vec.size()
             ? result
             : (result.push_back(f(vec[index])),
                fmapVectorRecursive<T, Func, U>(vec, f, index + 1,
                                                std::move(result)));
}
} // namespace detail

/** User Story: As a core fp functor consumer, I need to invoke fmap through a stable signature so the core fp functor workflow remains explicit and composable. @fn template <typename T, typename Func> auto fmap(const std::vector<T> &vec, Func f) -> std::vector<decltype(f(std::declval<const T &>()))> */
template <typename T, typename Func>
auto fmap(const std::vector<T> &vec, Func f)
    -> std::vector<decltype(f(std::declval<const T &>()))> {
  typedef decltype(f(std::declval<const T &>())) U;
  std::vector<U> result;
  result.reserve(vec.size());
  return detail::fmapVectorRecursive<T, Func, U>(vec, f, 0, std::move(result));
}

} // namespace func
