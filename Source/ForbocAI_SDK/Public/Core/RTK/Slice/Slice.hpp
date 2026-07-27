#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Slice/Builder.hpp"

namespace rtk {
namespace detail {
/**
 * @fn template <typename State> ActionReducerMapBuilder<State> createActionReducerMapBuilder(FString InName, State InInitialState)
 * @brief Constructs the reducer-map builder used by createSlice extraReducers.
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
 * @fn template <typename State> ReducedState<State> reduceCase(const TMap<FString, CaseReducer<State>> &ReducerMap, const State &PrevState, const AnyAction &Action)
 * @brief Evaluates an action against a specific case reducer if the type matches.
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
 * @fn template <typename State> ReducedState<State> reduceMatchersRecursive(const TArray<ActionMatcherDescription<State>> &ActionMatchers, const AnyAction &Action, int32 Index, ReducedState<State> Acc)
 * @brief Recursively evaluates an action against registered matcher reducers.
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
                   ActionMatchers, Action, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                   ActionMatchers[Index].Matcher(Action)
                       ? ReducedState<State>{
                             ActionMatchers[Index].Reducer(Acc.StateValue, Action),
                             true}
                       : Acc);
}

/**
 * @fn template <typename State> Slice<State> finalizeSlice(ActionReducerMapBuilder<State> Builder)
 * @brief Finalizes the slice and its reducer lookup table from a createSlice builder.
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
        reduceMatchersRecursive(ActionMatchers, Action, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
                                reduceCase(ReducerMap, PrevState, Action));

    return (!Reduced.bHandled && bHasDefaultCaseReducer)
               ? DefaultCaseReducer(Reduced.StateValue, Action)
               : Reduced.StateValue;
  };
  return Result;
}
} // namespace detail

/**
 * @fn template <typename State, typename ExtraReducersFn> Slice<State> createSlice(FString InName, State InInitialState, ExtraReducersFn ExtraReducers)
 * @brief Builds a slice from a name, initial state, and extraReducers callback.
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

/** User Story: As a core rtk slice consumer, I need to invoke build create slice through a stable signature so the core rtk slice workflow remains explicit and composable. @fn template <typename State> std::function<Slice<State>(FString, State, CaseReducer<State>)> buildCreateSlice() */
template <typename State>
std::function<Slice<State>(FString, State, CaseReducer<State>)>
buildCreateSlice() {
  return [](FString Name, State InitialState,
            CaseReducer<State> ReducerValue) {
    return createSlice<State>(MoveTemp(Name), MoveTemp(InitialState),
                              MoveTemp(ReducerValue));
  };
}

/**
 * @fn template <typename State, typename BuilderCallbackFn> CaseReducer<State> createReducer(State InitialState, BuilderCallbackFn BuilderCallback)
 * @brief Builds a reducer from initial state and an ActionReducerMapBuilder callback.
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
  return createSlice<State>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV220B8224F6F8), MoveTemp(InitialState),
                            MoveTemp(BuilderCallback))
      .Reducer;
}

/**
 * @fn template <typename State> Slice<State> createSlice(FString InName, State InInitialState, CaseReducer<State> ReducerFunc)
 * @brief Builds a slice around an already-composed reducer.
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
