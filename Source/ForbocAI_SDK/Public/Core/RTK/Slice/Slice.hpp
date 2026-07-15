#pragma once

#include "Core/RTK/Slice/Builder.hpp"

namespace rtk {
namespace detail {
/**
 * @brief Constructs the reducer-map builder used by createSlice extraReducers.
 * @signature template <typename State> ActionReducerMapBuilder<State> createActionReducerMapBuilder(FString InName, State InInitialState)
 * @param InName The slice name.
 * @param InInitialState The initial state for the builder.
 * @return ActionReducerMapBuilder<State> The uninitialized builder.
 *
 * User Story: As slice authors, I need an ActionReducerMapBuilder entry point
 * so extraReducers can register addCase handlers with RTK terminology.
 */
template <typename State>
ActionReducerMapBuilder<State>
createActionReducerMapBuilder(FString InName, State InInitialState) {
  ActionReducerMapBuilder<State> Builder;
  Builder.Name = MoveTemp(InName);
  Builder.InitialState = MoveTemp(InInitialState);
  return Builder;
}
} // namespace detail

/**
 * Finalizes the slice and its reducer lookup table from a createSlice builder.
 * User Story: As createSlice, I need one internal finalization path so slices
 * expose the generated reducer while callers only use RTK terms.
 */
namespace detail {
template <typename State> struct ReducedState {
  State StateValue;
  bool bHandled;
};

/**
 * @brief Evaluates an action against a specific case reducer if the type matches.
 * @signature template <typename State> ReducedState<State> reduceCase(const TMap<FString, CaseReducer<State>> &ReducerMap, const State &PrevState, const AnyAction &Action)
 * @param ReducerMap Map of action types to case reducers.
 * @param PrevState Current state before reduction.
 * @param Action Action being dispatched.
 * @return ReducedState<State> Resulting state and a boolean indicating if it was handled.
 *
 * User Story: As a functional store implementer, I need this to efficiently resolve and apply exact action matches.
 */
template <typename State>
ReducedState<State> reduceCase(const TMap<FString, CaseReducer<State>> &ReducerMap,
                               const State &PrevState,
                               const AnyAction &Action) {
  const CaseReducer<State> *Found = ReducerMap.Find(Action.Type);
  return Found ? ReducedState<State>{(*Found)(PrevState, Action), true}
               : ReducedState<State>{PrevState, false};
}

/**
 * @brief Recursively evaluates an action against registered matcher reducers.
 * @signature template <typename State> ReducedState<State> reduceMatchersRecursive(const TArray<ActionMatcherDescription<State>> &ActionMatchers, const AnyAction &Action, int32 Index, ReducedState<State> Acc)
 * @param ActionMatchers List of matcher configurations.
 * @param Action Action being dispatched.
 * @param Index Current recursion index.
 * @param Acc Accumulated state and handled flag.
 * @return ReducedState<State> Final resulting state and handled flag.
 *
 * User Story: As a functional store implementer, I need this to evaluate fallthrough and multiple matchers without loops.
 */
template <typename State>
ReducedState<State>
reduceMatchersRecursive(const TArray<ActionMatcherDescription<State>> &ActionMatchers,
                        const AnyAction &Action, int32 Index,
                        ReducedState<State> Acc) {
  return Index >= ActionMatchers.Num()
             ? Acc
             : reduceMatchersRecursive(
                   ActionMatchers, Action, Index + 1,
                   ActionMatchers[Index].Matcher(Action)
                       ? ReducedState<State>{
                             ActionMatchers[Index].Reducer(Acc.StateValue, Action),
                             true}
                       : Acc);
}

/**
 * @brief Finalizes the slice and its reducer lookup table from a createSlice builder.
 * @signature template <typename State> Slice<State> finalizeSlice(ActionReducerMapBuilder<State> Builder)
 * @param Builder The configured map builder.
 * @return Slice<State> The completed slice containing the composite reducer.
 *
 * User Story: As createSlice, I need one internal finalization path so slices
 * expose the generated reducer while callers only use RTK terms.
 */
template <typename State>
Slice<State> finalizeSlice(ActionReducerMapBuilder<State> Builder) {
  Slice<State> Result;
  Result.Name = Builder.Name;
  Result.InitialState = Builder.InitialState;
  auto ReducerMap = Builder.Reducers;
  auto ActionMatchers = Builder.ActionMatchers;
  auto DefaultCaseReducer = Builder.DefaultCaseReducer;
  const bool bHasDefaultCaseReducer = Builder.bHasDefaultCaseReducer;

  Result.Reducer =
      [ReducerMap, ActionMatchers, DefaultCaseReducer,
       bHasDefaultCaseReducer](const State &PrevState,
                               const AnyAction &Action) -> State {
    ReducedState<State> Reduced =
        reduceMatchersRecursive(ActionMatchers, Action, 0,
                                reduceCase(ReducerMap, PrevState, Action));

    return (!Reduced.bHandled && bHasDefaultCaseReducer)
               ? DefaultCaseReducer(Reduced.StateValue, Action)
               : Reduced.StateValue;
  };
  return Result;
}
} // namespace detail

/**
 * @brief Builds a slice from a name, initial state, and extraReducers callback.
 * @signature template <typename State, typename ExtraReducersFn> Slice<State> createSlice(FString InName, State InInitialState, ExtraReducersFn ExtraReducers)
 * @param InName The name of the slice.
 * @param InInitialState The initial state object.
 * @param ExtraReducers Function receiving a builder to attach reducers.
 * @return Slice<State> The generated slice with composed reducer.
 *
 * User Story: As slice authors, I need a createSlice entry point so C++ slice
 * files mirror Redux Toolkit's createSlice and builder.addCase patterns.
 */
template <typename State, typename ExtraReducersFn>
Slice<State> createSlice(FString InName, State InInitialState,
                         ExtraReducersFn ExtraReducers) {
  ActionReducerMapBuilder<State> Builder =
      detail::createActionReducerMapBuilder<State>(MoveTemp(InName),
                                                   MoveTemp(InInitialState));
  ExtraReducers(Builder);
  return detail::finalizeSlice(std::move(Builder));
}

/**
 * @brief Builds a reducer from initial state and an ActionReducerMapBuilder callback.
 * @signature template <typename State, typename BuilderCallbackFn> CaseReducer<State> createReducer(State InitialState, BuilderCallbackFn BuilderCallback)
 * @param InitialState Initial state for the reducer.
 * @param BuilderCallback Function to configure case matchers.
 * @return CaseReducer<State> The composed reducer function.
 *
 * User Story: As reducer authors, I need createReducer terminology for focused
 * reducer cases that do not need to be exported as a named slice.
 */
template <typename State, typename BuilderCallbackFn>
CaseReducer<State> createReducer(State InitialState,
                                 BuilderCallbackFn BuilderCallback) {
  return createSlice<State>(TEXT("reducer"), MoveTemp(InitialState),
                            MoveTemp(BuilderCallback))
      .Reducer;
}

/**
 * @brief Builds a slice around an already-composed reducer.
 * @signature template <typename State> Slice<State> createSlice(FString InName, State InInitialState, CaseReducer<State> ReducerFunc)
 * @param InName Name of the slice.
 * @param InInitialState The initial state object.
 * @param ReducerFunc An already composed root reducer function.
 * @return Slice<State> A new slice packaging the given reducer.
 *
 * User Story: As root slice authors, I need createSlice to accept a composed
 * reducer so combineReducers output can still be exported through one slice.
 */
template <typename State>
Slice<State> createSlice(FString InName, State InInitialState,
                         CaseReducer<State> ReducerFunc) {
  Slice<State> Result;
  Result.Name = MoveTemp(InName);
  Result.InitialState = MoveTemp(InInitialState);
  Result.Reducer = MoveTemp(ReducerFunc);
  return Result;
}

/**
 * Phase 3: Entity Adapter
 * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
 */

} // namespace rtk
