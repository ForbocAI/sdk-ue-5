#pragma once

#include "Core/RTK/Prelude.hpp"

namespace rtk {
namespace detail {
/**
 * @brief Evaluates a selector combiner against each input selector in a tuple.
 * @signature template <typename Result, typename State, typename Combiner, typename InputTuple, size_t... Is> Result evaluateSelector(const State &state, Combiner &combiner, const InputTuple &inputs, func::seq<Is...>)
 * @param state The current state to evaluate against.
 * @param combiner The combinatorial function.
 * @param inputs A tuple of input selectors.
 * @param unused Sequence indices for unpacking.
 * @return Result The evaluated derived state.
 *
 * User Story: As memoized selectors, I need tuple-driven combiner evaluation
 * so composed selectors can reuse one generic implementation.
 */
template <typename Result, typename State, typename Combiner,
          typename InputTuple, size_t... Is>
Result evaluateSelector(const State &state, Combiner &combiner,
                        const InputTuple &inputs, func::seq<Is...>) {
  return combiner(std::get<Is>(inputs)(state)...);
}
} // namespace detail

/**
 * @brief Creates a memoized selector from input selectors and a combiner.
 * @signature template <typename State, typename Result, typename... InSelectors> std::function<Result(const State &)> createSelector(const std::tuple<InSelectors...> &inputSelectors, std::function<Result(decltype(std::declval<InSelectors>()(std::declval<const State &>()))...)> combiner)
 * @param inputSelectors A tuple of input selector functions.
 * @param combiner A function combining the outputs of the input selectors.
 * @return std::function<Result(const State &)> The memoized selector.
 *
 * User Story: As selector authors, I need memoized selector composition so
 * derived state only recomputes when its inputs change.
 */
template <typename State, typename Result, typename... InSelectors>
std::function<Result(const State &)> createSelector(
    const std::tuple<InSelectors...> &inputSelectors,
    std::function<
        Result(decltype(std::declval<InSelectors>()(
            std::declval<const State &>()))...)>
        combiner) {
  /**
   * Memoize the combiner using FP core's memoizeLast.
   * It will automatically track the tuple of inputs across calls.
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  auto memoizedCombiner = func::memoizeLast(combiner);

  return
      [inputSelectors, memoizedCombiner](const State &state) mutable -> Result {
        return detail::evaluateSelector<Result>(
            state, memoizedCombiner, inputSelectors,
            func::gen_seq<sizeof...(InSelectors)>());
      };
}

} // namespace rtk
