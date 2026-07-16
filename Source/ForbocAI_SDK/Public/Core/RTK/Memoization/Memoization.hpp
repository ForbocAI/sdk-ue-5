#pragma once

#include "Core/RTK/Immer/Immer.hpp"

namespace rtk {

/** User Story: As a core rtk memoization consumer, I need to invoke lru memoize through a stable signature so the core rtk memoization workflow remains explicit and composable. @fn template <typename Function> auto lruMemoize(Function Value) -> decltype(func::memoizeLast(Value)) */
template <typename Function>
auto lruMemoize(Function Value) -> decltype(func::memoizeLast(Value)) {
  return func::memoizeLast(Value);
}

/** User Story: As a core rtk memoization consumer, I need to invoke weak map memoize through a stable signature so the core rtk memoization workflow remains explicit and composable. @fn template <typename Function> auto weakMapMemoize(Function Value) -> decltype(func::memoizeLast(Value)) */
template <typename Function>
auto weakMapMemoize(Function Value) -> decltype(func::memoizeLast(Value)) {
  return func::memoizeLast(Value);
}

/** User Story: As a core rtk memoization consumer, I need to invoke create selector creator through a stable signature so the core rtk memoization workflow remains explicit and composable. @fn template <typename Memoize> Memoize createSelectorCreator(Memoize Value) */
template <typename Memoize>
Memoize createSelectorCreator(Memoize Value) {
  return Value;
}

/** User Story: As a core rtk memoization consumer, I need to invoke create draft safe selector through a stable signature so the core rtk memoization workflow remains explicit and composable. @fn template <typename State, typename Result, typename... InSelectors> std::function<Result(const State &)> createDraftSafeSelector( const std::tuple<InSelectors...> &InputSelectors, std::function<Result(decltype(std::declval<InSelectors>()( std::declval<const State &>()))...)> Combiner) */
template <typename State, typename Result, typename... InSelectors>
std::function<Result(const State &)> createDraftSafeSelector(
    const std::tuple<InSelectors...> &InputSelectors,
    std::function<Result(decltype(std::declval<InSelectors>()(
        std::declval<const State &>()))...)> Combiner) {
  return createSelector<State, Result>(InputSelectors, Combiner);
}

/** User Story: As a core rtk memoization consumer, I need to invoke create draft safe selector creator through a stable signature so the core rtk memoization workflow remains explicit and composable. @fn template <typename Memoize> Memoize createDraftSafeSelectorCreator(Memoize Value) */
template <typename Memoize>
Memoize createDraftSafeSelectorCreator(Memoize Value) {
  return createSelectorCreator(Value);
}

} // namespace rtk
