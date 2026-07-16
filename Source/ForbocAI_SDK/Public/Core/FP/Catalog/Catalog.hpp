#pragma once

#include "Core/FP/Sequence/Sequence.hpp"
#include "Core/FP/Combinator/Combinator.hpp"

namespace func {
/**
 * @brief Stores a typed compile-time catalog of values or functions.
 *
 * @signature template <typename... Values> struct Catalog
 *
 * User Story: As feature code, I need noun-changing entries to become data
 * in one reusable list shape instead of spawning request/factory families.
 */
template <typename... Values> struct Catalog {
  std::tuple<Values...> values;
};

/**
 * @fn template <typename... Values> Catalog<typename std::decay<Values>::type...> catalog(Values &&...values)
 * @brief Builds a typed catalog from values or functions.
 *
 *
 * User Story: As composition code, I need a small variadic builder so repeated
 * selectors, projectors, validators, and transforms can be folded uniformly.
 */
template <typename... Values>
Catalog<typename std::decay<Values>::type...> catalog(Values &&...values) {
  return Catalog<typename std::decay<Values>::type...>{
      std::make_tuple(std::forward<Values>(values)...)};
}

namespace detail {
/** User Story: As a core fp catalog consumer, I need to invoke tail tuple impl through a stable signature so the core fp catalog workflow remains explicit and composable. @fn template <typename Tuple, size_t... Indices> auto tailTupleImpl(const Tuple &values, seq<Indices...>) -> decltype(std::make_tuple(std::get<Indices + 1>(values)...)) */
template <typename Tuple, size_t... Indices>
auto tailTupleImpl(const Tuple &values, seq<Indices...>)
    -> decltype(std::make_tuple(std::get<Indices + 1>(values)...)) {
  return std::make_tuple(std::get<Indices + 1>(values)...);
}
} // namespace detail

/**
 * @fn template <typename Head, typename... Tail> std::tuple<Tail...> tail_tuple(const std::tuple<Head, Tail...> &values)
 * @brief Returns a tuple containing every value after the head.
 *
 *
 * User Story: As catalog recursion, I need one neutral tuple-tail primitive so
 * higher folds do not reimplement tuple slicing per domain.
 */
template <typename Head, typename... Tail>
std::tuple<Tail...> tail_tuple(const std::tuple<Head, Tail...> &values) {
  return detail::tailTupleImpl(values, gen_seq<sizeof...(Tail)>());
}

namespace detail {
/** User Story: As a core fp catalog consumer, I need to invoke fold tuple recursive through a stable signature so the core fp catalog workflow remains explicit and composable. @fn template <typename Acc, typename Step> Acc foldTupleRecursive(const std::tuple<> &, Acc acc, Step) */
template <typename Acc, typename Step>
Acc foldTupleRecursive(const std::tuple<> &, Acc acc, Step) {
  return acc;
}

/** User Story: As a core fp catalog consumer, I need to invoke fold tuple recursive through a stable signature so the core fp catalog workflow remains explicit and composable. @fn template <typename Head, typename... Tail, typename Acc, typename Step> Acc foldTupleRecursive(const std::tuple<Head, Tail...> &values, Acc acc, Step step) */
template <typename Head, typename... Tail, typename Acc, typename Step>
Acc foldTupleRecursive(const std::tuple<Head, Tail...> &values, Acc acc,
                       Step step) {
  return foldTupleRecursive(tail_tuple(values),
                            step(acc, std::get<0>(values)), step);
}

/** User Story: As a core fp catalog consumer, I need to invoke zip tuple fold recursive through a stable signature so the core fp catalog workflow remains explicit and composable. @fn template <typename Acc, typename Step> Acc zipTupleFoldRecursive(const std::tuple<> &, const std::tuple<> &, Acc acc, Step) */
template <typename Acc, typename Step>
Acc zipTupleFoldRecursive(const std::tuple<> &, const std::tuple<> &, Acc acc,
                          Step) {
  return acc;
}

/** User Story: As a core fp catalog consumer, I need to invoke zip tuple fold recursive through a stable signature so the core fp catalog workflow remains explicit and composable. @fn template <typename LeftHead, typename... LeftTail, typename RightHead, typename... RightTail, typename Acc, typename Step> Acc zipTupleFoldRecursive(const std::tuple<LeftHead, LeftTail...> &left, const std::tuple<RightHead, RightTail...> &right, Acc acc, Step step) */
template <typename LeftHead, typename... LeftTail, typename RightHead,
          typename... RightTail, typename Acc, typename Step>
Acc zipTupleFoldRecursive(const std::tuple<LeftHead, LeftTail...> &left,
                          const std::tuple<RightHead, RightTail...> &right,
                          Acc acc, Step step) {
  return zipTupleFoldRecursive(
      tail_tuple(left), tail_tuple(right),
      step(acc, std::get<0>(left), std::get<0>(right)), step);
}
} // namespace detail

/**
 * @fn template <typename... Values, typename Acc, typename Step> Acc fold_catalog(const Catalog<Values...> &values, Acc seed, Step step)
 * @brief Folds a catalog through one accumulator step.
 *
 *
 * User Story: As ECS and RTK adapter code, I need function catalogs to execute
 * through one reusable recursion instead of repeated named wrapper families.
 */
template <typename... Values, typename Acc, typename Step>
Acc fold_catalog(const Catalog<Values...> &values, Acc seed, Step step) {
  return detail::foldTupleRecursive(values.values, seed, step);
}

/**
 * @fn template <typename... LeftValues, typename... RightValues, typename Acc, typename Step> Acc zip_catalog_fold(const Catalog<LeftValues...> &left, const Catalog<RightValues...> &right, Acc seed, Step step)
 * @brief Folds two catalogs in lockstep through one accumulator step.
 *
 *
 * User Story: As projection code, paired selector/projector lists should be
 * registered as data and executed by one neutral fold.
 */
template <typename... LeftValues, typename... RightValues, typename Acc,
          typename Step>
Acc zip_catalog_fold(const Catalog<LeftValues...> &left,
                     const Catalog<RightValues...> &right, Acc seed,
                     Step step) {
  static_assert(sizeof...(LeftValues) == sizeof...(RightValues),
                "zip_catalog_fold requires equally sized catalogs");
  return detail::zipTupleFoldRecursive(left.values, right.values, seed, step);
}

/** User Story: As a core fp catalog consumer, I need to invoke example clamp channel through a stable signature so the core fp catalog workflow remains explicit and composable. @fn inline std::function<int(int)> example_clamp_channel() */
inline std::function<int(int)> example_clamp_channel() {
  return pipe3([](int value) { return std::max(0, value); },
               [](int value) { return std::min(255, value); }, identity<int>);
}

/** User Story: As a core fp catalog consumer, I need to invoke example fill ratio through a stable signature so the core fp catalog workflow remains explicit and composable. @fn inline std::function<float(const std::pair<float, float> &)> example_fill_ratio() */
inline std::function<float(const std::pair<float, float> &)>
example_fill_ratio() {
  return converge2(
      [](float current, float maxValue) {
        return current / std::max(maxValue, 1.192092896e-07F);
      },
      [](const std::pair<float, float> &reading) { return reading.first; },
      [](const std::pair<float, float> &reading) { return reading.second; });
}

/** User Story: As a core fp catalog consumer, I need to invoke example alive and visible through a stable signature so the core fp catalog workflow remains explicit and composable. @fn inline std::function<bool(const std::pair<bool, bool> &)> example_alive_and_visible() */
inline std::function<bool(const std::pair<bool, bool> &)>
example_alive_and_visible() {
  return both([](const std::pair<bool, bool> &flags) { return flags.first; },
              [](const std::pair<bool, bool> &flags) { return flags.second; });
}

} // namespace func
