#pragma once

#include "Core/RTK/Action/ActionCreator.hpp"
#include "Core/RTK/Batch/AutoBatchTypes.hpp"
#include "Core/RTK/ConfigureStore/ConfigureStore.hpp"
#include "Core/RTK/Entity/Adapter.hpp"
#include "Core/RTK/Listener/Listener.hpp"
#include "Core/RTK/Selector/Selector.hpp"
#include "Core/RTK/Thunk/Thunk.hpp"

namespace rtk {

enum class ReducerType { Reducer, PreparedReducer, AsyncThunk };

struct ActionCreatorInvariantMiddlewareOptions {
  TArray<FString> IgnoredActionTypes;
};

template <typename State>
using CaseReducers = TMap<FString, CaseReducer<State>>;

template <typename State>
using SliceCaseReducers = CaseReducers<State>;

template <typename State>
using CaseReducerActions = TMap<FString, ActionCreatorWithoutPayload>;

template <typename State>
using CaseReducerWithPrepare = CaseReducer<State>;

template <typename Value>
using Comparer = std::function<int32(const Value &, const Value &)>;

template <typename State> struct ConfigureStoreOptions {
  State PreloadedState;
  CaseReducer<State> ReducerValue;
  TArray<Middleware<State>> MiddlewareValues;
  bool bDevTools = true;
};

template <typename State> struct CreateSliceOptions {
  FString Name;
  State InitialState;
  CaseReducers<State> Reducers;
  CaseReducer<State> ExtraReducers;
};

struct DevToolsEnhancerOptions {
  FString Name;
  bool bTrace = false;
  int32 TraceLimit = 75;
};

template <typename Value> using Draft = Value;
template <typename Value> using WritableDraft = Value;

using EntityId = FString;

template <typename Entity>
using EntityStateAdapter = EntityAdapter<Entity>;

template <typename Entity>
using IdSelector = std::function<EntityId(const Entity &)>;

template <typename Entity> struct Update {
  EntityId Id;
  std::function<Entity(const Entity &)> Changes;
};

template <typename State>
struct ImmutableStateInvariantMiddlewareOptions {
  std::function<bool(const State &)> IsImmutable;
  float WarnAfterSeconds = 0.032f;
  TArray<FString> IgnoredPaths;
};

struct SerializableStateInvariantMiddlewareOptions {
  float WarnAfterSeconds = 0.032f;
  TArray<FString> IgnoredActions;
  TArray<FString> IgnoredActionPaths;
  TArray<FString> IgnoredPaths;
};

template <typename State>
using Selector = std::function<State(const State &)>;

template <typename State, typename Result>
using OutputSelector = std::function<Result(const State &)>;

template <typename State>
using ReducerCreators = ActionReducerMapBuilder<State>;

template <typename State>
using SliceSelectors = TMap<FString, std::function<FString(const State &)>>;

template <typename State>
using ValidateSliceCaseReducers = CaseReducers<State>;

template <typename SliceT> using WithSlice = SliceT;
template <typename SliceT> using WithSlicePreloadedState = SliceT;

template <typename Result> using SafePromise = func::AsyncResult<Result>;

struct SerializedError {
  FString Name;
  FString Message;
  FString Stack;
  FString Code;
};

template <typename Result, typename Arg, typename State>
using AsyncThunk = AsyncThunkConfig<Result, Arg, State>;

template <typename Result, typename State>
using AsyncThunkAction = ThunkAction<Result, State>;

template <typename State> using AsyncThunkDispatchConfig = ThunkApi<State>;

template <typename Arg, typename State> struct AsyncThunkOptions {
  ConditionCallback<Arg, State> Condition;
  bool bDispatchConditionRejection = false;
};

template <typename Result, typename Arg, typename State>
using AsyncThunkPayloadCreator =
    std::function<func::AsyncResult<Result>(const Arg &,
                                            const ThunkApi<State> &)>;

template <typename Result>
using AsyncThunkPayloadCreatorReturnValue = func::AsyncResult<Result>;

template <typename Result, typename Arg, typename State>
using CreateAsyncThunkFunction =
    std::function<AsyncThunkConfig<Result, Arg, State>(
        const FString &, AsyncThunkPayloadCreator<Result, Arg, State>)>;

template <typename State> using GetDispatch = Dispatcher;
template <typename State> using GetState = std::function<const State &()>;
template <typename State> using GetThunkAPI = ThunkApi<State>;
template <typename State> using ThunkDispatch = Dispatcher;
template <typename State> using ThunkMiddleware = Middleware<State>;
template <typename State> using TSHelpersExtractDispatchExtensions = Dispatcher;

template <typename State>
using ListenerEffect =
    typename ListenerMiddleware<State>::EffectCallback;

template <typename State> using ListenerEffectAPI = MiddlewareApi<State>;
using ListenerErrorHandler = std::function<void(const FString &)>;

template <typename State>
using ListenerMiddlewareInstance = ListenerMiddleware<State>;

template <typename State> using MiddlewareApiConfig = MiddlewareApi<State>;

using AnyListenerPredicate = Matcher;

template <typename State>
using TypedStartListening = std::function<ListenerMiddleware<State>(
    ListenerMiddleware<State>, const FString &,
    typename ListenerMiddleware<State>::EffectCallback)>;

template <typename State>
using TypedAddListener = TypedStartListening<State>;

template <typename State>
using TypedStopListening = std::function<ListenerMiddleware<State>(
    ListenerMiddleware<State>, const FString &)>;

template <typename State>
using TypedRemoveListener = TypedStopListening<State>;

using UnsubscribeListener = std::function<void()>;

struct UnsubscribeListenerOptions {
  bool bCancelActive = false;
};

template <typename Result>
using AsyncTaskExecutor = std::function<func::AsyncResult<Result>()>;

template <typename Result> using SyncTaskExecutor = std::function<Result()>;

template <typename Result> struct TaskResolved { Result Value; };
struct TaskCancelled {};
struct TaskRejected { FString Error; };

template <typename Result> struct TaskResult {
  func::Maybe<TaskResolved<Result>> Resolved;
  func::Maybe<TaskRejected> Rejected;
  bool bCancelled = false;
};

template <typename Result> struct ForkedTask {
  SafePromise<TaskResult<Result>> ResultValue;
  std::function<void()> Cancel;
};

template <typename State> using ForkedTaskAPI = MiddlewareApi<State>;

template <typename Result, typename State>
using ForkedTaskExecutor =
    std::function<SafePromise<Result>(const ForkedTaskAPI<State> &)>;

template <typename State> struct CreateListenerMiddlewareOptions {
  State Extra;
  ListenerErrorHandler OnError;
};

template <typename State> using DynamicDispatch = Dispatcher;
template <typename State> using AddMiddleware = std::function<void(
    const Middleware<State> &)>;

} // namespace rtk
