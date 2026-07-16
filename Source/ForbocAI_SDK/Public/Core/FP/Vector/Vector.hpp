#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "Core/FP/Either/Either.hpp"

namespace func {
namespace detail {
/** User Story: As a core fp vector consumer, I need to invoke fold vector recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename T, typename Acc, typename Step> Acc foldVectorRecursive(const std::vector<T> &values, size_t index, Acc acc, Step step) */
template <typename T, typename Acc, typename Step>
Acc foldVectorRecursive(const std::vector<T> &values, size_t index, Acc acc,
                        Step step) {
  return index == values.size()
             ? acc
             : foldVectorRecursive<T, Acc, Step>(
                   values, index + 1, step(acc, values[index]), step);
}

/** User Story: As a core fp vector consumer, I need to invoke filter vector recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename T, typename Predicate> std::vector<T> filterVectorRecursive(const std::vector<T> &values, size_t index, Predicate predicate, std::vector<T> result) */
template <typename T, typename Predicate>
std::vector<T> filterVectorRecursive(const std::vector<T> &values,
                                     size_t index, Predicate predicate,
                                     std::vector<T> result) {
  return index == values.size()
             ? result
             : (predicate(values[index]) ? result.push_back(values[index])
                                         : void(),
                filterVectorRecursive<T, Predicate>(values, index + 1,
                                                    predicate,
                                                    std::move(result)));
}

/** User Story: As a core fp vector consumer, I need to invoke find vector recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename T, typename Predicate> Maybe<T> findVectorRecursive(const std::vector<T> &values, size_t index, Predicate predicate) */
template <typename T, typename Predicate>
Maybe<T> findVectorRecursive(const std::vector<T> &values, size_t index,
                             Predicate predicate) {
  return index == values.size()
             ? nothing<T>()
             : (predicate(values[index])
                    ? just(values[index])
                    : findVectorRecursive<T, Predicate>(values, index + 1,
                                                       predicate));
}

/** User Story: As a core fp vector consumer, I need to invoke vector contains key through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename Key> bool vectorContainsKey(const std::vector<Key> &values, const Key &key, size_t index) */
template <typename Key>
bool vectorContainsKey(const std::vector<Key> &values, const Key &key,
                       size_t index) {
  return index == values.size()
             ? false
             : (values[index] == key ||
                vectorContainsKey<Key>(values, key, index + 1));
}

/** User Story: As a core fp vector consumer, I need to invoke unique by recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename T, typename Projection, typename Key> std::vector<T> uniqueByRecursive(const std::vector<T> &values, size_t index, Projection projection, std::vector<Key> seen, std::vector<T> result) */
template <typename T, typename Projection, typename Key>
std::vector<T> uniqueByRecursive(const std::vector<T> &values, size_t index,
                                 Projection projection,
                                 std::vector<Key> seen,
                                 std::vector<T> result) {
  return index == values.size()
             ? result
             : (vectorContainsKey<Key>(seen, projection(values[index]), 0)
                    ? uniqueByRecursive<T, Projection, Key>(
                          values, index + 1, projection, std::move(seen),
                          std::move(result))
                    : (seen.push_back(projection(values[index])),
                       result.push_back(values[index]),
                       uniqueByRecursive<T, Projection, Key>(
                           values, index + 1, projection, std::move(seen),
                           std::move(result))));
}

/** User Story: As a core fp vector consumer, I need to invoke traverse maybe recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename T, typename Func, typename U> Maybe<std::vector<U>> traverseMaybeRecursive(const std::vector<T> &values, size_t index, Func fn, std::vector<U> result) */
template <typename T, typename Func, typename U>
Maybe<std::vector<U>> traverseMaybeRecursive(const std::vector<T> &values,
                                             size_t index, Func fn,
                                             std::vector<U> result) {
  return index == values.size()
             ? just(result)
             : [&]() {
                 Maybe<U> current = fn(values[index]);
                 return current.hasValue
                            ? (result.push_back(current.value),
                               traverseMaybeRecursive<T, Func, U>(
                                   values, index + 1, fn, std::move(result)))
                            : nothing<std::vector<U>>();
               }();
}

/** User Story: As a core fp vector consumer, I need to invoke fold either recursive through a stable signature so the core fp vector workflow remains explicit and composable. @fn template <typename E, typename T, typename Acc, typename Step> Either<E, Acc> foldEitherRecursive(const std::vector<T> &values, size_t index, Acc acc, Step step) */
template <typename E, typename T, typename Acc, typename Step>
Either<E, Acc> foldEitherRecursive(const std::vector<T> &values, size_t index,
                                   Acc acc, Step step) {
  return index == values.size()
             ? make_right<E, Acc>(acc)
             : ebind(step(acc, values[index]),
                     [&](const Acc &next) -> Either<E, Acc> {
                       return foldEitherRecursive<E, T, Acc, Step>(
                           values, index + 1, next, step);
                     });
}
} // namespace detail

/**
 * @fn template <typename T, typename Acc, typename Step> Acc fold(const std::vector<T> &values, Acc seed, Step step)
 * @brief Folds a vector through a pure accumulator step.
 *
 *
 * User Story: As feature and ECS code, I need a reusable fold so repeated
 * accumulation does not require domain-specific request wrappers.
 */
template <typename T, typename Acc, typename Step>
Acc fold(const std::vector<T> &values, Acc seed, Step step) {
  return detail::foldVectorRecursive<T, Acc, Step>(values, 0, seed, step);
}

/**
 * @fn template <typename T, typename Predicate> std::vector<T> filter(const std::vector<T> &values, Predicate predicate)
 * @brief Keeps vector values that satisfy a predicate.
 *
 *
 * User Story: As selectors and ECS queries, I need reusable filtering so
 * feature code can compose predicates instead of hand-writing branches.
 */
template <typename T, typename Predicate>
std::vector<T> filter(const std::vector<T> &values, Predicate predicate) {
  return detail::filterVectorRecursive<T, Predicate>(values, 0, predicate,
                                                     std::vector<T>());
}

/**
 * @fn template <typename T, typename Predicate> Maybe<T> find_vector(const std::vector<T> &values, Predicate predicate)
 * @brief Finds the first vector value that satisfies a predicate.
 *
 *
 * User Story: As lookup code, I need Maybe-returning search so absence remains
 * explicit and no default sentinel is required.
 */
template <typename T, typename Predicate>
Maybe<T> find_vector(const std::vector<T> &values, Predicate predicate) {
  return detail::findVectorRecursive<T, Predicate>(values, 0, predicate);
}

/**
 * @fn template <typename T> bool contains_value(const std::vector<T> &values, const T &expected)
 * @brief Returns true when a vector contains a value by equality.
 *
 *
 * User Story: As predicate code, I need value membership as a neutral helper
 * instead of duplicating index loops across domains.
 */
template <typename T>
bool contains_value(const std::vector<T> &values, const T &expected) {
  return detail::vectorContainsKey<T>(values, expected, 0);
}

/**
 * @fn template <typename T, typename Projection> std::vector<T> unique_by(const std::vector<T> &values, Projection projection)
 * @brief Keeps the first value for each projected key.
 *
 *
 * User Story: As derived-data code, I need uniqueness by a reusable projection
 * so selectors and ECS queries avoid one-off dedupe helpers.
 */
template <typename T, typename Projection>
std::vector<T> unique_by(const std::vector<T> &values, Projection projection) {
  typedef typename std::decay<decltype(projection(std::declval<const T &>()))>::
      type Key;
  return detail::uniqueByRecursive<T, Projection, Key>(
      values, 0, projection, std::vector<Key>(), std::vector<T>());
}

/**
 * @brief Maps each vector value through a Maybe-returning function.
 *
 * @signature template <typename T, typename Func> auto traverse(const std::vector<T> &values, Func fn) -> Maybe<std::vector<decltype(fn(std::declval<const T &>()).value)>>
 *
 * User Story: As JSON, ECS, and adapter code, I need all-or-nothing traversal
 * so incomplete records short-circuit without hidden defaults.
 */
template <typename T, typename Func>
auto traverse(const std::vector<T> &values, Func fn)
    -> Maybe<std::vector<decltype(fn(std::declval<const T &>()).value)>> {
  typedef decltype(fn(std::declval<const T &>()).value) U;
  return detail::traverseMaybeRecursive<T, Func, U>(values, 0, fn,
                                                   std::vector<U>());
}

/**
 * @fn template <typename T> Maybe<std::vector<T>> sequence_maybe(const std::vector<Maybe<T>> &values)
 * @brief Converts a vector of Maybe values into a Maybe vector.
 *
 *
 * User Story: As composition code, I need to collect optional values only when
 * every element exists so validation remains explicit.
 */
template <typename T>
Maybe<std::vector<T>> sequence_maybe(const std::vector<Maybe<T>> &values) {
  return traverse(values, [](const Maybe<T> &value) { return value; });
}

/**
 * @fn template <typename E, typename T, typename Acc, typename Step> Either<E, Acc> fold_either(const std::vector<T> &values, Acc seed, Step step)
 * @brief Folds a vector through an Either-returning accumulator step.
 *
 *
 * User Story: As validation and reducer-boundary code, I need accumulation to
 * stop on the first error while preserving successful accumulator values.
 */
template <typename E, typename T, typename Acc, typename Step>
Either<E, Acc> fold_either(const std::vector<T> &values, Acc seed, Step step) {
  return detail::foldEitherRecursive<E, T, Acc, Step>(values, 0, seed, step);
}

} // namespace func
