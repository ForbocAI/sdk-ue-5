#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

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
   * @fn template <typename Payload, typename ReducerFn> ActionReducerMapBuilder<State> & addCase(const ActionCreator<Payload> &Creator, ReducerFn ReducerFunc)
   */
  template <typename Payload, typename ReducerFn>
  ActionReducerMapBuilder<State> &
  addCase(const ActionCreator<Payload> &Creator, ReducerFn ReducerFunc) {
    checkf(!bMatcherAdded,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA515F0094D83));
    checkf(!bHasDefaultCaseReducer,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4A58257F5025));
    checkf(!Creator.Type.IsEmpty(),
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV92D2253C96F7));
    checkf(!Reducers.Contains(Creator.Type),
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV001A69F96B39));
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
   * @fn template <typename ReducerFn> ActionReducerMapBuilder<State> & addCase(const ActionCreatorWithoutPayload &Creator, ReducerFn ReducerFunc)
   */
  template <typename ReducerFn>
  ActionReducerMapBuilder<State> &
  addCase(const ActionCreatorWithoutPayload &Creator, ReducerFn ReducerFunc) {
    checkf(!bMatcherAdded,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA515F0094D83));
    checkf(!bHasDefaultCaseReducer,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4A58257F5025));
    checkf(!Creator.Type.IsEmpty(),
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV92D2253C96F7));
    checkf(!Reducers.Contains(Creator.Type),
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV001A69F96B39));
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
   * @fn ActionReducerMapBuilder<State> & addMatcher(std::function<bool(const AnyAction &)> Matcher, CaseReducer<State> ReducerFunc)
   */
  ActionReducerMapBuilder<State> &
  addMatcher(std::function<bool(const AnyAction &)> Matcher,
             CaseReducer<State> ReducerFunc) {
    checkf(!bHasDefaultCaseReducer,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEE0FAA62B4D0));
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
   * @fn ActionReducerMapBuilder<State> &addDefaultCase(CaseReducer<State> ReducerFunc)
   */
  ActionReducerMapBuilder<State> &addDefaultCase(CaseReducer<State> ReducerFunc) {
    checkf(!bHasDefaultCaseReducer,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEE3821C535BD));
    bHasDefaultCaseReducer = true;
    DefaultCaseReducer = std::move(ReducerFunc);
    return *this;
  }

  /** User Story: As a core rtk slice consumer, I need to invoke add async thunk through a stable signature so the core rtk slice workflow remains explicit and composable. @fn template <typename Returned, typename ThunkArg> ActionReducerMapBuilder<State> & addAsyncThunk(const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk, const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers) */
  template <typename Returned, typename ThunkArg>
  ActionReducerMapBuilder<State> &
  addAsyncThunk(const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk,
                const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers);
};

} // namespace rtk
