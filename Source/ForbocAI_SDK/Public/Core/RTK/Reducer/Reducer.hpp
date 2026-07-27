#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Store/Store.hpp"

namespace rtk {
template <typename RootState>
using CombinedSliceReducer = CaseReducer<RootState>;

template <typename RootState> struct ReducersMapObject {
  std::vector<
      std::function<bool(RootState &, const RootState &, const AnyAction &)>>
      Reducers;

  /** User Story: As a core rtk reducer consumer, I need to invoke reducer through a stable signature so the core rtk reducer workflow remains explicit and composable. @fn template <typename SliceState> ReducersMapObject<RootState> & reducer(SliceState RootState::*Member, CaseReducer<SliceState> ReducerFunc) */
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
/** User Story: As a core rtk reducer consumer, I need to invoke combine reducer entries recursive through a stable signature so the core rtk reducer workflow remains explicit and composable. @fn template <typename RootState> RootState combineReducerEntriesRecursive( const std::vector< std::function<bool(RootState &, const RootState &, const AnyAction &)>> &Reducers, size_t Index, const RootState &PrevState, RootState NextState, bool bChanged, const AnyAction &Action) */
template <typename RootState>
RootState combineReducerEntriesRecursive(
    const std::vector<
        std::function<bool(RootState &, const RootState &, const AnyAction &)>>
        &Reducers,
    size_t Index, const RootState &PrevState, RootState NextState,
    bool bChanged, const AnyAction &Action);

/**
 * @fn template <typename RootState> RootState combineReducerEntryStep( const std::vector< std::function<bool(RootState &, const RootState &, const AnyAction &)>> &Reducers, size_t Index, const RootState &PrevState, RootState NextState, bool bChanged, const AnyAction &Action)
 * @brief Combines a single reducer entry with the next state.
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
      Reducers, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, PrevState, std::move(NextState), bNextChanged,
      Action);
}

/**
 * @fn template <typename RootState> RootState combineReducerEntriesRecursive( const std::vector< std::function<bool(RootState &, const RootState &, const AnyAction &)>> &Reducers, size_t Index, const RootState &PrevState, RootState NextState, bool bChanged, const AnyAction &Action)
 * @brief Recursively iterates over all slice reducers.
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
 * @fn template <typename RootState> CaseReducer<RootState> combineReducers(const ReducersMapObject<RootState> &ReducersMap)
 */
template <typename RootState>
CaseReducer<RootState>
combineReducers(const ReducersMapObject<RootState> &ReducersMap) {
  auto Reducers = ReducersMap.Reducers;
  return [Reducers](const RootState &PrevState,
                    const AnyAction &Action) -> RootState {
    return detail::combineReducerEntriesRecursive<RootState>(
        Reducers, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, PrevState, PrevState, false, Action);
  };
}

/** User Story: As a core rtk reducer consumer, I need to invoke combine slices through a stable signature so the core rtk reducer workflow remains explicit and composable. @fn template <typename RootState> CombinedSliceReducer<RootState> combineSlices(const ReducersMapObject<RootState> &ReducersMap) */
template <typename RootState>
CombinedSliceReducer<RootState>
combineSlices(const ReducersMapObject<RootState> &ReducersMap) {
  return combineReducers(ReducersMap);
}

/**
 * 2.1 createAction<P> and Matchers
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
} // namespace rtk
