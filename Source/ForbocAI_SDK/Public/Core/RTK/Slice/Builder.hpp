#pragma once

#include "Core/RTK/Matcher/Matcher.hpp"

namespace rtk {
template <typename State> struct Slice {
  FString Name;
  State InitialState;
  CaseReducer<State> Reducer;
};

/**
 * 2.3 ActionReducerMapBuilder for createSlice extraReducers
 * User Story: As a maintainer, I need Redux Toolkit naming here so C++ slice
 * files map cleanly to createSlice and extraReducers builder.addCase patterns.
 */
template <typename State> struct ActionMatcherDescription {
  std::function<bool(const AnyAction &)> Matcher;
  CaseReducer<State> Reducer;
};

template <typename State, typename ThunkArg, typename Returned>
struct AsyncThunkReducers {
  bool bHasPending = false;
  bool bHasRejected = false;
  bool bHasFulfilled = false;
  bool bHasSettled = false;

  std::function<State(const State &, const PayloadAction<ThunkArg> &)> Pending;
  std::function<State(const State &, const PayloadAction<FString> &)> Rejected;
  std::function<State(const State &, const PayloadAction<Returned> &)> Fulfilled;
  CaseReducer<State> Settled;
};

template <typename State> struct ActionReducerMapBuilder {
  FString Name;
  State InitialState;
  TMap<FString, CaseReducer<State>> Reducers;
  TArray<ActionMatcherDescription<State>> ActionMatchers;
  CaseReducer<State> DefaultCaseReducer;
  bool bHasDefaultCaseReducer = false;
  bool bMatcherAdded = false;

  /**
   * Registers a typed reducer case for an existing action creator.
   * User Story: As slice authors, I need builder.addCase semantics so C++
   * slices mirror Redux Toolkit extraReducers callbacks.
   */
  template <typename Payload, typename ReducerFn>
  ActionReducerMapBuilder<State> &
  addCase(const ActionCreator<Payload> &Creator, ReducerFn ReducerFunc) {
    checkf(!bMatcherAdded,
           TEXT("builder.addCase should only be called before builder.addMatcher"));
    checkf(!bHasDefaultCaseReducer,
           TEXT("builder.addCase should only be called before builder.addDefaultCase"));
    checkf(!Creator.Type.IsEmpty(),
           TEXT("builder.addCase cannot be called with an empty action type"));
    checkf(!Reducers.Contains(Creator.Type),
           TEXT("builder.addCase cannot be called with two reducers for the same action type"));
    std::function<State(const State &, const PayloadAction<Payload> &)>
        WrappedReducer = ReducerFunc;
    Reducers.Add(
        Creator.Type,
        [Creator, WrappedReducer](const State &PrevState,
                                  const AnyAction &AnyActionValue) -> State {
          auto PayloadOpt = Creator.extract(AnyActionValue);
          return PayloadOpt.hasValue
                     ? WrappedReducer(
                           PrevState,
                           detail::payloadAction(AnyActionValue.Type,
                                                 PayloadOpt.value))
                     : PrevState;
        });
    return *this;
  }

  /**
   * Registers an empty-payload reducer case for an existing action creator.
   * User Story: As slice authors, I need builder.addCase for lifecycle actions
   * that carry no domain payload.
   */
  template <typename ReducerFn>
  ActionReducerMapBuilder<State> &
  addCase(const ActionCreatorWithoutPayload &Creator, ReducerFn ReducerFunc) {
    checkf(!bMatcherAdded,
           TEXT("builder.addCase should only be called before builder.addMatcher"));
    checkf(!bHasDefaultCaseReducer,
           TEXT("builder.addCase should only be called before builder.addDefaultCase"));
    checkf(!Creator.Type.IsEmpty(),
           TEXT("builder.addCase cannot be called with an empty action type"));
    checkf(!Reducers.Contains(Creator.Type),
           TEXT("builder.addCase cannot be called with two reducers for the same action type"));
    std::function<State(const State &, const PayloadAction<FEmptyPayload> &)>
        WrappedReducer = ReducerFunc;
    Reducers.Add(
        Creator.Type,
        [Creator, WrappedReducer](const State &PrevState,
                                  const AnyAction &AnyActionValue) -> State {
          return Creator.match(AnyActionValue)
                     ? WrappedReducer(PrevState,
                                      detail::payloadAction(AnyActionValue.Type))
                     : PrevState;
        });
    return *this;
  }

  /**
   * Registers a matcher reducer for action predicates.
   * User Story: As createReducer/createSlice users, I need addMatcher semantics
   * so multiple matching reducers can run after exact addCase reducers.
   */
  ActionReducerMapBuilder<State> &
  addMatcher(std::function<bool(const AnyAction &)> Matcher,
             CaseReducer<State> ReducerFunc) {
    checkf(!bHasDefaultCaseReducer,
           TEXT("builder.addMatcher should only be called before builder.addDefaultCase"));
    bMatcherAdded = true;
    ActionMatchers.Add(ActionMatcherDescription<State>{
        std::move(Matcher), std::move(ReducerFunc)});
    return *this;
  }

  /**
   * Registers the default reducer for otherwise-unhandled actions.
   * User Story: As createReducer/createSlice users, I need addDefaultCase
   * semantics so default-case behavior only runs when no case or matcher handled
   * the action.
   */
  ActionReducerMapBuilder<State> &addDefaultCase(CaseReducer<State> ReducerFunc) {
    checkf(!bHasDefaultCaseReducer,
           TEXT("builder.addDefaultCase can only be called once"));
    bHasDefaultCaseReducer = true;
    DefaultCaseReducer = std::move(ReducerFunc);
    return *this;
  }

  template <typename Returned, typename ThunkArg>
  ActionReducerMapBuilder<State> &
  addAsyncThunk(const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk,
                const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers);
};

} // namespace rtk
