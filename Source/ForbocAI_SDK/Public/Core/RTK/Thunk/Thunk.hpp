#pragma once

#include "Core/RTK/Slice/Slice.hpp"

namespace rtk {
template <typename State> struct ThunkApi {
  std::function<AnyAction(const AnyAction &)> dispatch;
  std::function<const State &()> getState;
};

template <typename Result, typename State>
using ThunkAction = std::function<func::AsyncResult<Result>(
    std::function<AnyAction(const AnyAction &)>,
    std::function<const State &()>)>;

/**
 * @brief Callback type for the createAsyncThunk condition guard.
 * Returning false skips thunk execution (no pending/fulfilled dispatched).
 */
template <typename Arg, typename State>
using ConditionCallback =
    std::function<bool(const Arg &, const ThunkApi<State> &)>;

template <typename Result, typename Arg, typename State>
struct AsyncThunkConfig {
  FString TypePrefix;
  ActionCreator<Arg> pending;
  ActionCreator<Result> fulfilled;
  ActionCreator<FString> rejected;

  std::function<ThunkAction<Result, State>(const Arg &)> thunkActionCreator;

  /** User Story: As a core rtk thunk consumer, I need to invoke the callable value through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn ThunkAction<Result, State> operator()(const Arg &arg) const */
  ThunkAction<Result, State> operator()(const Arg &arg) const {
    return thunkActionCreator(arg);
  }
};

/** User Story: As a core rtk thunk consumer, I need to invoke create async thunk through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename Result, typename Arg, typename State> AsyncThunkConfig<Result, Arg, State> createAsyncThunk( const FString &TypePrefix, std::function<func::AsyncResult<Result>(const Arg &, const ThunkApi<State> &)> PayloadCreator) */
template <typename Result, typename Arg, typename State>
AsyncThunkConfig<Result, Arg, State> createAsyncThunk(
    const FString &TypePrefix,
    std::function<func::AsyncResult<Result>(const Arg &,
                                            const ThunkApi<State> &)>
        PayloadCreator);

struct AsyncThunkCreator {
  /** User Story: As a core rtk thunk consumer, I need to invoke the callable value through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename Result, typename Arg, typename State> AsyncThunkConfig<Result, Arg, State> operator()( const FString &TypePrefix, std::function<func::AsyncResult<Result>(const Arg &, const ThunkApi<State> &)> PayloadCreator) const */
  template <typename Result, typename Arg, typename State>
  AsyncThunkConfig<Result, Arg, State> operator()(
      const FString &TypePrefix,
      std::function<func::AsyncResult<Result>(const Arg &,
                                              const ThunkApi<State> &)>
          PayloadCreator) const {
    return createAsyncThunk<Result, Arg, State>(TypePrefix, PayloadCreator);
  }
};

static const AsyncThunkCreator asyncThunkCreator = AsyncThunkCreator();

/** User Story: As a core rtk thunk consumer, I need to invoke unwrap result through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename Error, typename Result> Result unwrapResult(const func::Either<Error, Result> &Value) */
template <typename Error, typename Result>
Result unwrapResult(const func::Either<Error, Result> &Value) {
  checkf(!Value.isLeft, TEXT("unwrapResult received a rejected result"));
  return Value.right;
}

namespace detail {
/**
 * Wraps an already available value in a resolved AsyncResult.
 * User Story: As thunk authors, I need synchronous values adapted into
 * AsyncResult so feature workflows can compose through the RTK async contract.
 * @fn template <typename T> inline func::AsyncResult<T> ResolveAsync(const T &Value)
 */
template <typename T>
inline func::AsyncResult<T> ResolveAsync(const T &Value) {
  return func::createAsyncResult<T>(
      [Value](std::function<void(T)> Resolve,
              std::function<void(std::string)> Reject) { Resolve(Value); });
}

/**
 * Wraps a UE string error in a rejected AsyncResult.
 * User Story: As thunk authors, I need UE-string failures adapted into
 * AsyncResult so feature workflows can reject through the RTK async contract.
 * @fn template <typename T> inline func::AsyncResult<T> RejectAsync(const FString &Error)
 */
template <typename T>
inline func::AsyncResult<T> RejectAsync(const FString &Error) {
  const std::string Utf8Error = TCHAR_TO_UTF8(*Error);
  return func::createAsyncResult<T>(
      [Utf8Error](std::function<void(T)> Resolve,
                  std::function<void(std::string)> Reject) {
        Reject(Utf8Error);
      });
}

/** User Story: As a core rtk thunk consumer, I need to invoke add async thunk case when through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename State, typename CreatorT, typename ReducerT> ActionReducerMapBuilder<State> & addAsyncThunkCaseWhen(ActionReducerMapBuilder<State> &Builder, bool bHasReducer, const CreatorT &Creator, const ReducerT &Reducer) */
template <typename State, typename CreatorT, typename ReducerT>
ActionReducerMapBuilder<State> &
addAsyncThunkCaseWhen(ActionReducerMapBuilder<State> &Builder,
                      bool bHasReducer, const CreatorT &Creator,
                      const ReducerT &Reducer) {
  return bHasReducer ? Builder.addCase(Creator, Reducer) : Builder;
}

/** User Story: As a core rtk thunk consumer, I need to invoke add async thunk settled matcher when through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename State, typename Returned, typename ThunkArg> ActionReducerMapBuilder<State> &addAsyncThunkSettledMatcherWhen( ActionReducerMapBuilder<State> &Builder, bool bHasSettled, const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk, const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers) */
template <typename State, typename Returned, typename ThunkArg>
ActionReducerMapBuilder<State> &addAsyncThunkSettledMatcherWhen(
    ActionReducerMapBuilder<State> &Builder, bool bHasSettled,
    const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk,
    const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers) {
  const ActionCreator<Returned> Fulfilled = AsyncThunk.fulfilled;
  const ActionCreator<FString> Rejected = AsyncThunk.rejected;
  return bHasSettled
             ? Builder.addMatcher(
                   [Fulfilled, Rejected](const AnyAction &ActionValue) {
                     return Fulfilled.match(ActionValue) ||
                            Rejected.match(ActionValue);
                   },
                   Reducers.Settled)
             : Builder;
}
} // namespace detail

/**
 * @fn template <typename Result, typename Arg, typename State> AsyncThunkConfig<Result, Arg, State> createAsyncThunk( const FString &TypePrefix, std::function<func::AsyncResult<Result>(const Arg &, const ThunkApi<State> &)> PayloadCreator, ConditionCallback<Arg, State> Condition)
 * @brief Creates a thunk config with pending, fulfilled, and rejected lifecycle
 * actions, plus an optional condition guard.
 * @param TypePrefix The prefix string used for action types.
 * @param PayloadCreator The async function returning an AsyncResult.
 * @param Condition Optional guard - return false to skip execution.
 * @return AsyncThunkConfig<Result, Arg, State> The thunk configuration.
 *
 * User Story: As async thunk authors, I need lifecycle action wiring generated
 * automatically so pending and result dispatch stay consistent.
 *
 * RTK-THUNK-004: The condition callback is evaluated before dispatching
 * pending. When it returns false the thunk returns a rejected AsyncResult
 * ("Aborted: condition returned false") with no lifecycle actions dispatched,
 * preventing StrictMode double-invokes and stale duplicate requests.
 */
template <typename Result, typename Arg, typename State>
AsyncThunkConfig<Result, Arg, State> createAsyncThunk(
    const FString &TypePrefix,
    std::function<func::AsyncResult<Result>(const Arg &,
                                            const ThunkApi<State> &)>
        PayloadCreator,
    ConditionCallback<Arg, State> Condition) {
  auto pending = createAction<Arg>(TypePrefix + TEXT("/pending"));
  auto fulfilled = createAction<Result>(TypePrefix + TEXT("/fulfilled"));
  auto rejected = createAction<FString>(TypePrefix + TEXT("/rejected"));

  auto thunkActionCreator = [pending, fulfilled, rejected, PayloadCreator,
                             Condition](
                                const Arg &arg) -> ThunkAction<Result, State> {
    return [pending, fulfilled, rejected, PayloadCreator, Condition,
            arg](std::function<AnyAction(const AnyAction &)> dispatch,
                 std::function<const State &()> getState)
                 -> func::AsyncResult<Result> {
      /**
       * 0. Evaluate condition guard (RTK-THUNK-004)
       * User Story: As a thunk author, I need a condition guard so
       * StrictMode double-invokes and stale requests are skipped at the
       * thunk level before any lifecycle actions are dispatched.
       */
      ThunkApi<State> conditionApi{dispatch, getState};
      const bool bConditionAllowsRun =
          Condition ? Condition(arg, conditionApi) : true;
      return !bConditionAllowsRun
                 ? func::createAsyncResult<Result>(
                       [](std::function<void(Result)>,
                          std::function<void(std::string)> Reject) {
                         Reject("Aborted: condition returned false");
                       })
                 : [&]() -> func::AsyncResult<Result> {

                     /**
                      * 1. Dispatch pending synchronously
                      * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
                      */
                     dispatch(pending(arg));

                     /**
                      * 2. Build the ThunkApi surface
                      * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                      */
                     ThunkApi<State> api{dispatch, getState};

                     /**
                      * 3. Execute payload creator
                      * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                      */
                     auto result = PayloadCreator(arg, api);

                     /**
                      * 4. Chain lifecycle actions using FP core AsyncChain.
                      *    The returned AsyncResult carries both the fulfilled chain and
                      *    the catch_ error handler. The caller is responsible for calling
                      *    .execute() on the returned result to trigger the full chain.
                      * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
                      */
                     func::AsyncResult<Result> Chained =
                         func::AsyncChain::then<Result, Result>(
                             result, [dispatch, fulfilled](Result res) {
                               dispatch(fulfilled(res));
                               return func::createAsyncResult<Result>(
                                   [res](std::function<void(Result)> resolve,
                                         std::function<void(std::string)> reject) {
                                     resolve(res);
                                   });
                             });
                     func::catchAsync(Chained,
                                      [dispatch, rejected](std::string err) {
                                        dispatch(rejected(FString(
                                            UTF8_TO_TCHAR(err.c_str()))));
                                      });
                     return Chained;
                   }();
    };
  };

  return AsyncThunkConfig<Result, Arg, State>{TypePrefix, pending, fulfilled,
                                              rejected, thunkActionCreator};
}

/**
 * @fn template <typename Result, typename Arg, typename State> AsyncThunkConfig<Result, Arg, State> createAsyncThunk( const FString &TypePrefix, std::function<func::AsyncResult<Result>(const Arg &, const ThunkApi<State> &)> PayloadCreator)
 * @brief Convenience overload without a condition guard (always proceeds).
 * User Story: As a core rtk thunk consumer, I need to invoke create async thunk through a stable signature so the core rtk thunk workflow remains explicit and composable.
 */
template <typename Result, typename Arg, typename State>
AsyncThunkConfig<Result, Arg, State> createAsyncThunk(
    const FString &TypePrefix,
    std::function<func::AsyncResult<Result>(const Arg &,
                                            const ThunkApi<State> &)>
        PayloadCreator) {
  return createAsyncThunk<Result, Arg, State>(
      TypePrefix, PayloadCreator, ConditionCallback<Arg, State>{});
}

/** User Story: As a core rtk thunk consumer, I need to invoke add async thunk through a stable signature so the core rtk thunk workflow remains explicit and composable. @fn template <typename State> template <typename Returned, typename ThunkArg> ActionReducerMapBuilder<State> &ActionReducerMapBuilder<State>::addAsyncThunk( const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk, const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers) */
template <typename State>
template <typename Returned, typename ThunkArg>
ActionReducerMapBuilder<State> &ActionReducerMapBuilder<State>::addAsyncThunk(
    const AsyncThunkConfig<Returned, ThunkArg, State> &AsyncThunk,
    const AsyncThunkReducers<State, ThunkArg, Returned> &Reducers) {
  checkf(!bHasDefaultCaseReducer,
         TEXT("builder.addAsyncThunk should only be called before builder.addDefaultCase"));

  return detail::addAsyncThunkSettledMatcherWhen(
      detail::addAsyncThunkCaseWhen(
          detail::addAsyncThunkCaseWhen(
              detail::addAsyncThunkCaseWhen(*this, Reducers.bHasPending,
                                            AsyncThunk.pending,
                                            Reducers.Pending),
              Reducers.bHasRejected, AsyncThunk.rejected, Reducers.Rejected),
          Reducers.bHasFulfilled, AsyncThunk.fulfilled, Reducers.Fulfilled),
      Reducers.bHasSettled, AsyncThunk, Reducers);
}

/**
 * Phase 5: Middleware
 * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
 */

} // namespace rtk
