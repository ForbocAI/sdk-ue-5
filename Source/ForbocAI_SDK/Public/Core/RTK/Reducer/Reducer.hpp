#pragma once

#include "Core/RTK/Store/Store.hpp"

namespace rtk {
template <typename RootState> struct ReducersMapObject {
  std::vector<
      std::function<bool(RootState &, const RootState &, const AnyAction &)>>
      Reducers;

  template <typename SliceState>
  ReducersMapObject<RootState> &
  reducer(SliceState RootState::*Member, CaseReducer<SliceState> ReducerFunc) {
    Reducers.push_back(
        [Member, ReducerFunc](RootState &NextState,
                              const RootState &PrevState,
                              const AnyAction &Action) {
          const SliceState &PrevSlice = PrevState.*Member;
          SliceState NextSlice = ReducerFunc(PrevSlice, Action);
          bool bChanged = !(PrevSlice == NextSlice);
          return bChanged ? (NextState.*Member = std::move(NextSlice), true)
                          : false;
        });
    return *this;
  }
};

namespace detail {
template <typename RootState>
RootState combineReducerEntriesRecursive(
    const std::vector<
        std::function<bool(RootState &, const RootState &, const AnyAction &)>>
        &Reducers,
    size_t Index, const RootState &PrevState, RootState NextState,
    bool bChanged, const AnyAction &Action);

/**
 * @brief Combines a single reducer entry with the next state.
 * @signature template <typename RootState> RootState combineReducerEntryStep(...)
 * @param Reducers List of reducer functions.
 * @param Index Current index in the list.
 * @param PrevState Previous root state.
 * @param NextState Accumulated root state.
 * @param bChanged Flag indicating if state has changed.
 * @param Action Action being processed.
 * @return RootState The new accumulated root state.
 *
 * User Story: As a functional store implementer, I need this to evaluate a single combineReducers step purely.
 */
template <typename RootState>
RootState combineReducerEntryStep(
    const std::vector<
        std::function<bool(RootState &, const RootState &, const AnyAction &)>>
        &Reducers,
    size_t Index, const RootState &PrevState, RootState NextState,
    bool bChanged, const AnyAction &Action) {
  const bool bNextChanged =
      Reducers[Index](NextState, PrevState, Action) ? true : bChanged;
  return combineReducerEntriesRecursive<RootState>(
      Reducers, Index + 1, PrevState, std::move(NextState), bNextChanged,
      Action);
}

/**
 * @brief Recursively iterates over all slice reducers.
 * @signature template <typename RootState> RootState combineReducerEntriesRecursive(...)
 * @param Reducers List of reducer functions.
 * @param Index Current index in the list.
 * @param PrevState Previous root state.
 * @param NextState Accumulated root state.
 * @param bChanged Flag indicating if state has changed.
 * @param Action Action being processed.
 * @return RootState The final root state after all reducers have run.
 *
 * User Story: As a functional store implementer, I need this to recursively apply all combineReducers entries without loops.
 */
template <typename RootState>
RootState combineReducerEntriesRecursive(
    const std::vector<
        std::function<bool(RootState &, const RootState &, const AnyAction &)>>
        &Reducers,
    size_t Index, const RootState &PrevState, RootState NextState,
    bool bChanged, const AnyAction &Action) {
  return Index == Reducers.size()
             ? (bChanged ? NextState : PrevState)
             : combineReducerEntryStep<RootState>(
                   Reducers, Index, PrevState, std::move(NextState), bChanged,
                   Action);
}
} // namespace detail

/**
 * Produces a root reducer that calls every reducer in the reducers map object.
 * User Story: As root-store assembly, I need Redux combineReducers semantics so
 * slice reducers receive the same dispatched action coherently.
 */
template <typename RootState>
CaseReducer<RootState>
combineReducers(const ReducersMapObject<RootState> &ReducersMap) {
  auto Reducers = ReducersMap.Reducers;
  return [Reducers](const RootState &PrevState,
                    const AnyAction &Action) -> RootState {
    return detail::combineReducerEntriesRecursive<RootState>(
        Reducers, 0, PrevState, PrevState, false, Action);
  };
}

/**
 * 2.1 createAction<P> and Matchers
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
} // namespace rtk
