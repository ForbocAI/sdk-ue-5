#pragma once
#ifndef RTK_HPP
#define RTK_HPP

#include "CoreMinimal.h"
#include "ue_fp.hpp"
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace rtk {

struct FEmptyPayload;

namespace payload_debug {

/**
 * @brief Converts an FEmptyPayload to a debug string.
 * @signature FString DebugPayloadString(const FEmptyPayload &)
 * @param Payload The empty payload (unused).
 * @return FString A string representation indicating no payload.
 *
 * User Story: As a debugger, I need this function to visually identify when an action carries no payload.
 */
inline FString DebugPayloadString(const FEmptyPayload &) { return TEXT("<none>"); }

/**
 * @brief Returns the FString payload as a debug string.
 * @signature FString DebugPayloadString(const FString &Value)
 * @param Value The FString payload.
 * @return FString The FString payload itself.
 *
 * User Story: As a debugger, I need this function to easily read FString payloads directly in the debug output.
 */
inline FString DebugPayloadString(const FString &Value) { return Value; }

/**
 * @brief Converts a boolean payload to a debug string.
 * @signature FString DebugPayloadString(const bool &Value)
 * @param Value The boolean payload.
 * @return FString "true" if the value is true, "false" otherwise.
 *
 * User Story: As a debugger, I need this function to clearly see the true/false state of boolean payloads.
 */
inline FString DebugPayloadString(const bool &Value) {
  return Value ? TEXT("true") : TEXT("false");
}

/**
 * @brief Converts an integral payload (non-boolean) to a debug string.
 * @signature template <typename T> typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The integral payload.
 * @return FString The integral value formatted as a string.
 *
 * User Story: As a debugger, I need this function to reliably inspect integer payloads of various sizes in a uniform string format.
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value &&
                            !std::is_same<T, bool>::value,
                        FString>::type
DebugPayloadString(const T &Value) {
  return FString::Printf(TEXT("%lld"), static_cast<long long>(Value));
}

/**
 * @brief Converts a floating-point payload to a debug string.
 * @signature template <typename T> typename std::enable_if<std::is_floating_point<T>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The floating-point payload.
 * @return FString The floating-point value safely converted to a string.
 *
 * User Story: As a debugger, I need this function to verify decimal precision or floating-point states without truncation.
 */
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, FString>::type
DebugPayloadString(const T &Value) {
  return FString::SanitizeFloat(static_cast<double>(Value));
}

template <typename T> class HasToString {
  template <typename U>
  static auto Test(int) -> decltype(std::declval<const U &>().ToString(),
                                    std::true_type());

  template <typename> static std::false_type Test(...);

public:
  static const bool value = decltype(Test<T>(0))::value;
};

/**
 * @brief Converts a payload with a ToString() method to a debug string.
 * @signature template <typename T> typename std::enable_if<HasToString<T>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The payload object.
 * @return FString The string representation obtained from the object's ToString() method.
 *
 * User Story: As a debugger, I need this function so that complex objects can define their own string representations automatically.
 */
template <typename T>
typename std::enable_if<HasToString<T>::value, FString>::type
DebugPayloadString(const T &Value) {
  return Value.ToString();
}

/**
 * @brief Converts a TArray payload to a debug string indicating its length.
 * @signature template <typename T> FString DebugPayloadString(const TArray<T> &Values)
 * @param Values The TArray payload.
 * @return FString A string showing the length of the array.
 *
 * User Story: As a debugger, I need this function to quickly gauge the size of array payloads without overwhelming the log output.
 */
template <typename T>
FString DebugPayloadString(const TArray<T> &Values) {
  return FString::Printf(TEXT("TArray(len=%d)"), Values.Num());
}

/**
 * @brief Converts a TMap payload to a debug string indicating its length.
 * @signature template <typename K, typename V> FString DebugPayloadString(const TMap<K, V> &Map)
 * @param Map The TMap payload.
 * @return FString A string showing the number of entries in the map.
 *
 * User Story: As a debugger, I need this function to quickly assess the size of map payloads in store actions.
 */
template <typename K, typename V> FString DebugPayloadString(const TMap<K, V> &Map) {
  return FString::Printf(TEXT("TMap(len=%d)"), Map.Num());
}

/**
 * @brief Default conversion for payloads that cannot be easily stringified.
 * @signature template <typename T> typename std::enable_if<!HasToString<T>::value && !std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, FString>::type DebugPayloadString(const T &)
 * @param unused The opaque payload.
 * @return FString A placeholder string "<opaque>".
 *
 * User Story: As a debugger, I need this function to gracefully handle unknown types without causing compilation errors when actions are dispatched.
 */
template <typename T>
typename std::enable_if<!HasToString<T>::value &&
                            !std::is_integral<T>::value &&
                            !std::is_floating_point<T>::value &&
                            !std::is_same<T, bool>::value,
                        FString>::type
DebugPayloadString(const T &) {
  return TEXT("<opaque>");
}

} // namespace payload_debug

/**
 * 1.1 Action<Payload>
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename Payload> struct Action {
  FString Type;
  Payload PayloadValue;
};

template <typename Payload> using PayloadAction = Action<Payload>;

struct FEmptyPayload {};

namespace detail {
/**
 * @brief Helper to create a PayloadAction with a specific value.
 * @signature template <typename Payload> PayloadAction<Payload> payloadAction(const FString &Type, const Payload &PayloadValue)
 * @param Type The action type string.
 * @param PayloadValue The actual payload data.
 * @return PayloadAction<Payload> The created typed action.
 *
 * User Story: As a developer writing reducers, I need this function to easily construct strongly-typed actions with data.
 */
template <typename Payload>
PayloadAction<Payload> payloadAction(const FString &Type,
                                     const Payload &PayloadValue) {
  return PayloadAction<Payload>{Type, PayloadValue};
}

/**
 * @brief Helper to create a PayloadAction with an empty payload.
 * @signature inline PayloadAction<FEmptyPayload> payloadAction(const FString &Type)
 * @param Type The action type string.
 * @return PayloadAction<FEmptyPayload> The created empty action.
 *
 * User Story: As a developer writing reducers, I need this function to easily construct strongly-typed actions representing pure events.
 */
inline PayloadAction<FEmptyPayload> payloadAction(const FString &Type) {
  return PayloadAction<FEmptyPayload>{Type, FEmptyPayload{}};
}
} // namespace detail

/**
 * Type-erased envelope for heterogeneous root dispatch
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
struct AnyAction {
  FString Type;
  std::shared_ptr<void> PayloadWrapper;
  FString PayloadDebugText;

  /**
   * Constructs an empty type-erased action envelope.
   * User Story: As root dispatch infrastructure, I need a default AnyAction so
   * containers and return paths can be initialized before payload assignment.
   */
  AnyAction() : PayloadDebugText(TEXT("<none>")) {}

  /**
   * Constructs a type-erased action envelope from a type tag and payload wrapper.
   * User Story: As root dispatch infrastructure, I need a type-erased action
   * constructor so heterogeneous payloads can move through one dispatch channel.
   */
  AnyAction(const FString &InType, std::shared_ptr<void> InPayloadWrapper,
            const FString &InPayloadDebugText = TEXT("<opaque>"))
      : Type(InType),
        PayloadWrapper(std::move(InPayloadWrapper)),
        PayloadDebugText(InPayloadDebugText) {}

  /**
   * Extracts a typed payload from the type-erased storage.
   * User Story: As root dispatch consumers, I need a direct payload accessor so
   * infrastructure code can recover stored action data when type ownership is known.
   * Warning: This performs an unchecked static_cast. Callers must ensure the requested
   * payload type matches the stored value. Prefer ActionCreator::extract() when possible.
   */
  template <typename Payload> func::Maybe<Payload> getPayload() const {
    return PayloadWrapper
               ? func::just(*static_cast<Payload *>(PayloadWrapper.get()))
               : func::nothing<Payload>();
  }

  FString describePayload() const {
    return PayloadDebugText.IsEmpty() ? TEXT("<none>") : PayloadDebugText;
  }
};

using UnknownAction = AnyAction;

/**
 * 1.2 Reducer
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
template <typename State, typename ActionT>
using Reducer = std::function<State(const State &, const ActionT &)>;

template <typename State>
using CaseReducer = std::function<State(const State &, const AnyAction &)>;

template <typename State> struct ActionReducerMapBuilder;
template <typename Result, typename Arg, typename State> struct AsyncThunkConfig;

template <typename State> struct Store;

template <typename State>
Store<State> createStore(State InitialState, CaseReducer<State> ReducerFunc);

template <typename State> const State &getState(const Store<State> &StoreValue);

template <typename State>
AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action);

template <typename State>
std::function<void()> subscribe(Store<State> &StoreValue,
                                std::function<void()> Callback);

/**
 * 1.3 Store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename State> struct Store {
  State CurrentState;
  CaseReducer<State> RootReducer;

  struct Subscriber {
    int64_t Id;
    std::function<void()> Callback;
  };
  std::vector<Subscriber> Subscribers;
  int64_t NextId = 1;

  /**
   * Returns the current store state snapshot.
   * User Story: As store consumers, I need the current state exposed so
   * dispatchers, selectors, and subscribers can inspect runtime data.
   */
  const State &getState() const { return rtk::getState(*this); }

  /**
   * Applies an action and notifies subscribers after the reducer runs.
   * User Story: As store consumers, I need dispatch to update state and notify
   * listeners so runtime flows can react to reducer changes.
   */
  AnyAction dispatch(const AnyAction &action) {
    return rtk::dispatch(*this, action);
  }

  /**
   * Registers a callback and returns an unsubscribe function.
   * User Story: As store consumers, I need subscriptions with unsubscribe
   * handles so runtime code can observe state safely.
   */
  std::function<void()> subscribe(std::function<void()> Callback) {
    return rtk::subscribe(*this, std::move(Callback));
  }
};

namespace detail {
/**
 * @brief Recursively notifies all subscribers in the list.
 * @signature template <typename State> void notifySubscribersRecursive(const std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index)
 * @param Subscribers The list of active subscribers.
 * @param Index The current recursion index.
 *
 * User Story: As a functional store implementer, I need this to notify all listeners immutably without loops.
 */
template <typename State>
void notifySubscribersRecursive(
    const std::vector<typename Store<State>::Subscriber> &Subscribers,
    size_t Index) {
  Index == Subscribers.size()
      ? void()
      : (Subscribers[Index].Callback(),
         notifySubscribersRecursive<State>(Subscribers, Index + 1));
}

/**
 * @brief Erases a subscriber at a specific index.
 * @signature template <typename State> void eraseSubscriberAt(std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index)
 * @param Subscribers The list of subscribers to modify.
 * @param Index The index to remove.
 *
 * User Story: As a functional store implementer, I need this to safely remove an unsubscribed listener.
 */
template <typename State>
void eraseSubscriberAt(std::vector<typename Store<State>::Subscriber> &Subscribers,
                       size_t Index) {
  Subscribers.erase(Subscribers.begin() + Index);
}

/**
 * @brief Recursively finds and removes a subscriber by Id.
 * @signature template <typename State> void eraseSubscriberRecursive(std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index, int64_t Id)
 * @param Subscribers The list of subscribers.
 * @param Index The current recursion index.
 * @param Id The unique identifier of the subscriber to remove.
 *
 * User Story: As a functional store implementer, I need this to recursively scan and clean up subscriptions without loops.
 */
template <typename State>
void eraseSubscriberRecursive(
    std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index,
    int64_t Id) {
  Index == Subscribers.size()
      ? void()
      : (Subscribers[Index].Id == Id
             ? eraseSubscriberAt<State>(Subscribers, Index)
             : eraseSubscriberRecursive<State>(Subscribers, Index + 1, Id),
         void());
}
} // namespace detail

/**
 * @brief Creates a Redux-like store with an initial state and root reducer.
 * @signature template <typename State> Store<State> createStore(State InitialState, CaseReducer<State> ReducerFunc)
 * @param InitialState The initial state of the store.
 * @param ReducerFunc The root reducer function.
 * @return Store<State> The created store instance.
 *
 * User Story: As an app developer, I need this function to initialize my central state container.
 */
template <typename State>
Store<State> createStore(State InitialState, CaseReducer<State> ReducerFunc) {
  Store<State> StoreValue;
  StoreValue.CurrentState = std::move(InitialState);
  StoreValue.RootReducer = std::move(ReducerFunc);
  return StoreValue;
}

/**
 * @brief Gets the current state from a given store.
 * @signature template <typename State> const State &getState(const Store<State> &StoreValue)
 * @param StoreValue The store to query.
 * @return const State& A const reference to the current state.
 *
 * User Story: As a developer, I need this to read the current state synchronously from a store instance.
 */
template <typename State> const State &getState(const Store<State> &StoreValue) {
  return StoreValue.CurrentState;
}

/**
 * @brief Dispatches an action to a store, updating its state and notifying subscribers.
 * @signature template <typename State> AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action)
 * @param StoreValue The store to dispatch to.
 * @param Action The type-erased action to process.
 * @return AnyAction The dispatched action.
 *
 * User Story: As a developer, I need this function to trigger state updates predictably.
 */
template <typename State>
AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action) {
  StoreValue.CurrentState = StoreValue.RootReducer(StoreValue.CurrentState, Action);
  const std::vector<typename Store<State>::Subscriber> SubsCopy =
      StoreValue.Subscribers;
  detail::notifySubscribersRecursive<State>(SubsCopy, 0);
  return Action;
}

/**
 * @brief Subscribes a callback to state changes in the store.
 * @signature template <typename State> std::function<void()> subscribe(Store<State> &StoreValue, std::function<void()> Callback)
 * @param StoreValue The store to subscribe to.
 * @param Callback The function to call on state changes.
 * @return std::function<void()> An unsubscribe function to remove the callback.
 *
 * User Story: As a UI developer, I need to subscribe to the store so my views can re-render when state changes.
 */
template <typename State>
std::function<void()> subscribe(Store<State> &StoreValue,
                                std::function<void()> Callback) {
  const int64_t Id = StoreValue.NextId++;
  StoreValue.Subscribers.push_back(
      typename Store<State>::Subscriber{Id, std::move(Callback)});
  return [&StoreValue, Id]() {
    detail::eraseSubscriberRecursive<State>(StoreValue.Subscribers, 0, Id);
  };
}

/**
 * 1.4 combineReducers
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
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
template <typename Payload> struct ActionCreator {
  FString Type;

  AnyAction operator()(const Payload &payload) const {
    return AnyAction(Type, std::make_shared<Payload>(payload),
                     payload_debug::DebugPayloadString(payload));
  }

  /**
   * Reports whether an AnyAction matches this creator's type tag.
   * User Story: As reducer helpers, I need action-type matching so handlers can
   * confirm payload shape before extraction.
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }

  /**
   * Extracts a typed payload when the action type matches this creator.
   * User Story: As reducer helpers, I need safe payload extraction so typed
   * reducers can consume AnyAction without repeating casts.
   */
  func::Maybe<Payload> extract(const AnyAction &action) const {
    return match(action) ? action.getPayload<Payload>()
                         : func::nothing<Payload>();
  }
};

/**
 * @brief Creates a typed action creator for a namespaced action type.
 * @signature template <typename Payload> ActionCreator<Payload> createAction(const FString &Type)
 * @param Type The action type identifier string.
 * @return ActionCreator<Payload> The action creator function object.
 *
 * User Story: As slice code, I need typed action creators so reducers and
 * thunks can share stable action contracts.
 */
template <typename Payload>
ActionCreator<Payload> createAction(const FString &Type) {
  return ActionCreator<Payload>{Type};
}

struct ActionCreatorWithoutPayload {
  FString Type;

  AnyAction operator()() const {
    return AnyAction(Type, std::make_shared<FEmptyPayload>(),
                     payload_debug::DebugPayloadString(FEmptyPayload{}));
  }

  /**
   * Reports whether an AnyAction matches this empty action creator.
   * User Story: As reducer helpers, I need empty-action matching so lifecycle
   * actions can be recognized without custom payload structs.
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }
};

/**
 * @brief Creates an empty-payload action creator for a namespaced action type.
 * @signature inline ActionCreatorWithoutPayload createAction(const FString &Type)
 * @param Type The action type identifier string.
 * @return ActionCreatorWithoutPayload The empty payload action creator object.
 *
 * User Story: As slice code, I need empty action creators so simple lifecycle
 * events can reuse the same RTK-style creation pattern.
 */
inline ActionCreatorWithoutPayload createAction(const FString &Type) {
  return ActionCreatorWithoutPayload{Type};
}


/**
 * @brief Matcher alias for RTK-style action predicates.
 * @signature using Matcher = std::function<bool(const AnyAction &)>
 *
 * User Story: As reducer and listener authors, I need a reusable action
 * predicate type so exact actions, lifecycle suffixes, and composed predicates
 * share one vocabulary.
 */
using Matcher = std::function<bool(const AnyAction &)>;

/**
 * @brief Checks whether an action type ends with a lifecycle suffix.
 * @signature inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix)
 *
 * User Story: As async reducers, I need `/pending`, `/fulfilled`, and
 * `/rejected` suffix matching to follow Redux Toolkit lifecycle semantics.
 */
inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix) {
  return Action.Type.EndsWith(Suffix);
}

inline bool matchesThunkLifecycle(const AnyAction &Action, const FString &Suffix,
                                  const TArray<FString> &TypePrefixes) {
  return TypePrefixes.Num() == 0
             ? actionTypeEndsWith(Action, Suffix)
             : TypePrefixes.ContainsByPredicate(
                   [&Action, &Suffix](const FString &TypePrefix) {
                     return Action.Type == TypePrefix + Suffix;
                   });
}

/**
 * @brief Composes matchers with logical OR, equivalent to RTK isAnyOf.
 * @signature inline Matcher isAnyOf(const TArray<Matcher> &Matchers)
 *
 * User Story: As a reducer author, I need to group several event actions under
 * one state transition without setter-style action names.
 */
inline Matcher isAnyOf(const TArray<Matcher> &Matchers) {
  return [Matchers](const AnyAction &Action) {
    return Matchers.ContainsByPredicate(
        [&Action](const Matcher &Candidate) { return Candidate(Action); });
  };
}

/**
 * @brief Composes matchers with logical AND, equivalent to RTK isAllOf.
 * @signature inline Matcher isAllOf(const TArray<Matcher> &Matchers)
 *
 * User Story: As listener middleware, I need multiple predicates to agree
 * before a side effect runs.
 */
inline Matcher isAllOf(const TArray<Matcher> &Matchers) {
  return [Matchers](const AnyAction &Action) {
    return !Matchers.ContainsByPredicate(
        [&Action](const Matcher &Candidate) { return !Candidate(Action); });
  };
}

/** @brief Creates an RTK-style matcher for `/pending` async thunk actions. */
inline Matcher isPending(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/pending"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for `/fulfilled` async thunk actions. */
inline Matcher isFulfilled(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/fulfilled"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for `/rejected` async thunk actions. */
inline Matcher isRejected(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/rejected"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for any async thunk lifecycle action. */
inline Matcher isAsyncThunkAction(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  TArray<Matcher> Matchers;
  Matchers.Add(isPending(TypePrefixes));
  Matchers.Add(isFulfilled(TypePrefixes));
  Matchers.Add(isRejected(TypePrefixes));
  return isAnyOf(Matchers);
}

/**
 * 2.2 Slice<State>
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
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

template <typename T> struct EntityState {
  TArray<FString> ids;
  TMap<FString, T> entities;
};

namespace detail {
template <typename T>
bool entityStateValueEquals(const TMap<FString, T> &LeftEntities,
                            const TMap<FString, T> &RightEntities,
                            const FString &Id) {
  const T *LeftEntity = LeftEntities.Find(Id);
  const T *RightEntity = RightEntities.Find(Id);
  return LeftEntity && RightEntity && (*LeftEntity == *RightEntity);
}

template <typename T>
bool entityStateEqualsRecursive(const EntityState<T> &Left,
                                const EntityState<T> &Right, int32 Index) {
  return Index >= Left.ids.Num()
             ? true
             : Left.ids[Index] == Right.ids[Index] &&
                   entityStateValueEquals(Left.entities, Right.entities,
                                          Left.ids[Index]) &&
                   entityStateEqualsRecursive(Left, Right, Index + 1);
}
} // namespace detail

/**
 * @brief Checks if two EntityState objects are deeply equal.
 * @signature template <typename T> bool operator==(const EntityState<T> &Left, const EntityState<T> &Right)
 * @param Left The first state to compare.
 * @param Right The second state to compare.
 * @return true if both states have the same entities and ordering; false otherwise.
 *
 * User Story: As a functional reducer, I need deep equality checks to avoid unnecessary state updates when payload data matches existing state.
 */
template <typename T>
bool operator==(const EntityState<T> &Left, const EntityState<T> &Right) {
  return Left.ids.Num() == Right.ids.Num() &&
         Left.entities.Num() == Right.entities.Num() &&
         detail::entityStateEqualsRecursive(Left, Right, 0);
}

/**
 * @brief Checks if two EntityState objects are not equal.
 * @signature template <typename T> bool operator!=(const EntityState<T> &Left, const EntityState<T> &Right)
 * @param Left The first state to compare.
 * @param Right The second state to compare.
 * @return true if the states differ; false otherwise.
 *
 * User Story: As a functional reducer, I need inequality checks to trigger state changes when updating collections.
 */
template <typename T>
bool operator!=(const EntityState<T> &Left, const EntityState<T> &Right) {
  return !(Left == Right);
}

template <typename T> struct EntitySelectors {
  std::function<TArray<T>(const EntityState<T> &)> selectAll;
  std::function<func::Maybe<T>(const EntityState<T> &, const FString &)>
      selectById;
  std::function<TArray<FString>(const EntityState<T> &)> selectIds;
  std::function<int32_t(const EntityState<T> &)> selectTotal;
};

template <typename T> struct EntityAdapter;

namespace detail {
template <typename T>
void addEntityIfMissing(EntityState<T> &Next, const FString &Id,
                        const T &Entity) {
  const bool bMissing = !Next.entities.Find(Id);
  bMissing && (Next.ids.Add(Id), true);
  bMissing && (Next.entities.Add(Id, Entity), true);
}

template <typename T>
void setEntity(EntityState<T> &Next, const FString &Id, const T &Entity) {
  (!Next.entities.Find(Id)) && (Next.ids.Add(Id), true);
  Next.entities.Add(Id, Entity);
}

template <typename T>
void removeEntityIfPresent(EntityState<T> &Next, const FString &Id) {
  (Next.entities.Remove(Id) > 0) && (Next.ids.Remove(Id), true);
}

template <typename T, typename PatchFn>
void updateEntityIfPresent(EntityState<T> &Next, const FString &Id,
                           PatchFn Patch) {
  const T *Existing = Next.entities.Find(Id);
  Existing && (Next.entities.Add(Id, Patch(*Existing)), true);
}

template <typename T>
void appendEntityIfPresent(TArray<T> &Result, const EntityState<T> &State,
                           const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  Entity && (Result.Add(*Entity), true);
}

template <typename T>
func::Maybe<T> findEntityById(const EntityState<T> &State, const FString &Id) {
  const T *Entity = State.entities.Find(Id);
  return Entity ? func::just(*Entity) : func::nothing<T>();
}

template <typename T>
EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                        const TArray<T> &NewEntities,
                                        int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops,
                                       const TArray<T> &NewEntities,
                                       int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                           const TArray<T> &EntitiesToUpsert,
                                           int32 Index, EntityState<T> Next);

template <typename T>
EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds,
                                           int32 Index, EntityState<T> Next);

template <typename T>
TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index,
                                     TArray<T> Result);
} // namespace detail

template <typename T> struct EntityAdapter {
  std::function<FString(const T &)> selectId;

  /**
   * Returns an empty entity-state container.
   * User Story: As entity-backed slices, I need a canonical empty entity state
   * so adapters can initialize predictable reducer storage.
   */
  EntityState<T> getInitialState() const { return EntityState<T>{{}, {}}; }

  /**
   * Adds a single entity when its id is not already present.
   * User Story: As entity-backed slices, I need single-entity insertion so new
   * records can be added without mutating existing adapter state.
   */
  EntityState<T> addOne(const EntityState<T> &state, const T &entity) const {
    EntityState<T> next = state;
    FString id = selectId(entity);
    detail::addEntityIfMissing(next, id, entity);
    return next;
  }

  /**
   * Adds each missing entity from a batch without replacing existing entries.
   * User Story: As entity-backed slices, I need batch insertion so collections
   * can be seeded while preserving existing records.
   */
  EntityState<T> addMany(const EntityState<T> &state,
                         const TArray<T> &newEntities) const {
    return detail::addManyEntitiesRecursive(*this, newEntities, 0, state);
  }

  /**
   * Inserts or replaces a single entity by id.
   * User Story: As entity-backed slices, I need single-entity replacement so
   * reducers can upsert records deterministically.
   */
  EntityState<T> setOne(const EntityState<T> &state, const T &entity) const {
    EntityState<T> next = state;
    FString id = selectId(entity);
    detail::setEntity(next, id, entity);
    return next;
  }

  /**
   * Replaces the full entity set with the provided collection.
   * User Story: As entity-backed slices, I need whole-collection replacement so
   * reducers can resync adapter state from remote payloads.
   */
  EntityState<T> setAll(const EntityState<T> &state,
                        const TArray<T> &newEntities) const {
    return detail::setAllEntitiesRecursive(*this, newEntities, 0,
                                           EntityState<T>());
  }

  /**
   * Upserts a single entity by delegating to setOne.
   * User Story: As entity-backed slices, I need a semantic upsert helper so
   * reducers can express intent without duplicating adapter logic.
   */
  EntityState<T> upsertOne(const EntityState<T> &state, const T &entity) const {
    return setOne(state, entity);
  }

  /**
   * Upserts a batch of entities by id.
   * User Story: As entity-backed slices, I need batch upsert so synced payloads
   * can merge into adapter state efficiently.
   */
  EntityState<T> upsertMany(const EntityState<T> &state,
                            const TArray<T> &entitiesToUpsert) const {
    return detail::upsertManyEntitiesRecursive(*this, entitiesToUpsert, 0,
                                               state);
  }

  /**
   * Removes a single entity and its id when present.
   * User Story: As entity-backed slices, I need record removal so deleted items
   * disappear from both entity maps and id orderings.
   */
  EntityState<T> removeOne(const EntityState<T> &state,
                           const FString &id) const {
    EntityState<T> next = state;
    detail::removeEntityIfPresent(next, id);
    return next;
  }

  /**
   * Removes all entities whose ids appear in the supplied list.
   * User Story: As entity-backed slices, I need batch removal so reducers can
   * clear multiple records in one pure operation.
   */
  EntityState<T> removeMany(const EntityState<T> &state,
                            const TArray<FString> &removeIds) const {
    return detail::removeManyEntitiesRecursive(removeIds, 0, state);
  }

  /**
   * Clears every entity from the adapter state.
   * User Story: As entity-backed slices, I need a reset helper so adapters can
   * return to a clean initial state predictably.
   */
  EntityState<T> removeAll(const EntityState<T> &) const {
    return getInitialState();
  }

  /**
   * Replaces one entity with the result of a patch function.
   * User Story: As entity-backed slices, I need targeted patching so one record
   * can be updated without rebuilding the full collection manually.
   */
  EntityState<T> updateOne(const EntityState<T> &state, const FString &id,
                           std::function<T(const T &)> patch) const {
    EntityState<T> next = state;
    detail::updateEntityIfPresent(next, id, patch);
    return next;
  }

  /**
   * Builds selector helpers for the current adapter shape.
   * User Story: As entity-backed slices, I need selector helpers so callers can
   * read ids, entities, and totals without hand-rolled lookup code.
   */
  EntitySelectors<T> getSelectors() const {
    const auto SelectAll = [](const EntityState<T> &state) -> TArray<T> {
      return detail::selectAllEntitiesRecursive(state, 0, TArray<T>());
    };

    const auto SelectById =
        [](const EntityState<T> &state, const FString &id) -> func::Maybe<T> {
      return detail::findEntityById(state, id);
    };

    const auto SelectIds = [](const EntityState<T> &state) -> TArray<FString> {
      return state.ids;
    };

    const auto SelectTotal = [](const EntityState<T> &state) -> int32_t {
      return state.ids.Num();
    };

    return EntitySelectors<T>{
        SelectAll, SelectById, SelectIds, SelectTotal};
  }
};

namespace detail {
template <typename T>
EntityState<T> addManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                        const TArray<T> &NewEntities,
                                        int32 Index, EntityState<T> Next) {
  return Index >= NewEntities.Num()
             ? Next
             : (addEntityIfMissing(Next, Ops.selectId(NewEntities[Index]),
                                   NewEntities[Index]),
                addManyEntitiesRecursive(Ops, NewEntities, Index + 1,
                                         std::move(Next)));
}

template <typename T>
EntityState<T> setAllEntitiesRecursive(const EntityAdapter<T> &Ops,
                                       const TArray<T> &NewEntities,
                                       int32 Index, EntityState<T> Next) {
  return Index >= NewEntities.Num()
             ? Next
             : (Next.ids.Add(Ops.selectId(NewEntities[Index])),
                Next.entities.Add(Ops.selectId(NewEntities[Index]),
                                  NewEntities[Index]),
                setAllEntitiesRecursive(Ops, NewEntities, Index + 1,
                                        std::move(Next)));
}

template <typename T>
EntityState<T> upsertManyEntitiesRecursive(const EntityAdapter<T> &Ops,
                                           const TArray<T> &EntitiesToUpsert,
                                           int32 Index, EntityState<T> Next) {
  return Index >= EntitiesToUpsert.Num()
             ? Next
             : upsertManyEntitiesRecursive(Ops, EntitiesToUpsert, Index + 1,
                                           Ops.setOne(Next,
                                                      EntitiesToUpsert[Index]));
}

template <typename T>
EntityState<T> removeManyEntitiesRecursive(const TArray<FString> &RemoveIds,
                                           int32 Index, EntityState<T> Next) {
  return Index >= RemoveIds.Num()
             ? Next
             : (removeEntityIfPresent(Next, RemoveIds[Index]),
                removeManyEntitiesRecursive(RemoveIds, Index + 1,
                                            std::move(Next)));
}

template <typename T>
TArray<T> selectAllEntitiesRecursive(const EntityState<T> &State, int32 Index,
                                     TArray<T> Result) {
  return Index >= State.ids.Num()
             ? Result
             : (appendEntityIfPresent(Result, State, State.ids[Index]),
                selectAllEntitiesRecursive(State, Index + 1,
                                           std::move(Result)));
}
} // namespace detail

/**
 * @brief Creates entity-adapter operations from an id selector.
 * @signature template <typename T> EntityAdapter<T> createEntityAdapter(std::function<FString(const T &)> selectId)
 * @param selectId A function to extract the string ID from an entity.
 * @return EntityAdapter<T> The adapter with CRUD operation helpers.
 *
 * User Story: As slice authors, I need adapter factories so entity state
 * management can be generated from one id-selection rule.
 */
template <typename T>
EntityAdapter<T>
createEntityAdapter(std::function<FString(const T &)> selectId) {
  return EntityAdapter<T>{std::move(selectId)};
}

/**
 * Phase 4: Async Thunks
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename State> struct ThunkApi {
  std::function<AnyAction(const AnyAction &)> dispatch;
  std::function<const State &()> getState;
};

template <typename Result, typename State>
using ThunkAction = std::function<func::AsyncResult<Result>(
    std::function<AnyAction(const AnyAction &)>,
    std::function<const State &()>)>;

template <typename Result, typename Arg, typename State>
struct AsyncThunkConfig {
  FString TypePrefix;
  ActionCreator<Arg> pending;
  ActionCreator<Result> fulfilled;
  ActionCreator<FString> rejected;

  std::function<ThunkAction<Result, State>(const Arg &)> thunkActionCreator;

  ThunkAction<Result, State> operator()(const Arg &arg) const {
    return thunkActionCreator(arg);
  }
};

namespace detail {
template <typename State, typename CreatorT, typename ReducerT>
ActionReducerMapBuilder<State> &
addAsyncThunkCaseWhen(ActionReducerMapBuilder<State> &Builder,
                      bool bHasReducer, const CreatorT &Creator,
                      const ReducerT &Reducer) {
  return bHasReducer ? Builder.addCase(Creator, Reducer) : Builder;
}

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
 * @brief Creates a thunk config with pending, fulfilled, and rejected lifecycle actions.
 * @signature template <typename Result, typename Arg, typename State> AsyncThunkConfig<Result, Arg, State> createAsyncThunk(const FString &TypePrefix, std::function<func::AsyncResult<Result>(const Arg &, const ThunkApi<State> &)> PayloadCreator)
 * @param TypePrefix The prefix string used for action types.
 * @param PayloadCreator The async function returning an AsyncResult.
 * @return AsyncThunkConfig<Result, Arg, State> The thunk configuration.
 *
 * User Story: As async thunk authors, I need lifecycle action wiring generated
 * automatically so pending and result dispatch stay consistent.
 */
template <typename Result, typename Arg, typename State>
AsyncThunkConfig<Result, Arg, State> createAsyncThunk(
    const FString &TypePrefix,
    std::function<func::AsyncResult<Result>(const Arg &,
                                            const ThunkApi<State> &)>
        PayloadCreator) {
  auto pending = createAction<Arg>(TypePrefix + TEXT("/pending"));
  auto fulfilled = createAction<Result>(TypePrefix + TEXT("/fulfilled"));
  auto rejected = createAction<FString>(TypePrefix + TEXT("/rejected"));

  auto thunkActionCreator = [pending, fulfilled, rejected, PayloadCreator](
                                const Arg &arg) -> ThunkAction<Result, State> {
    return [pending, fulfilled, rejected, PayloadCreator,
            arg](std::function<AnyAction(const AnyAction &)> dispatch,
                 std::function<const State &()> getState)
                 -> func::AsyncResult<Result> {
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
      func::AsyncResult<Result> Chained = func::AsyncChain::then<Result, Result>(
          result, [dispatch, fulfilled](Result res) {
            dispatch(fulfilled(res));
            return func::createAsyncResult<Result>(
                [res](std::function<void(Result)> resolve,
                      std::function<void(std::string)> reject) {
                  resolve(res);
                });
          });
      func::catchAsync(Chained, [dispatch, rejected](std::string err) {
        dispatch(rejected(FString(UTF8_TO_TCHAR(err.c_str()))));
      });
      return Chained;
    };
  };

  return AsyncThunkConfig<Result, Arg, State>{TypePrefix, pending, fulfilled,
                                              rejected, thunkActionCreator};
}

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

template <typename State> struct MiddlewareApi {
  std::function<AnyAction(const AnyAction &)> dispatch;
  std::function<const State &()> getState;
};

/**
 * Dispatch function type used throughout the middleware chain.
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */
using Dispatcher = std::function<AnyAction(const AnyAction &)>;

template <typename State>
using Middleware =
    std::function<std::function<Dispatcher(Dispatcher)>(
        const MiddlewareApi<State> &)>;

namespace detail {
template <typename State>
Dispatcher applyMiddlewareRecursive(
    typename std::vector<Middleware<State>>::const_reverse_iterator It,
    typename std::vector<Middleware<State>>::const_reverse_iterator End,
    const MiddlewareApi<State> &Api, Dispatcher CurrentDispatch) {
  return It == End
             ? CurrentDispatch
             : applyMiddlewareRecursive<State>(It + 1, End, Api,
                                               (*It)(Api)(CurrentDispatch));
}
} // namespace detail

/**
 * @brief Wraps a dispatcher with middleware while preserving RTK-style composition order.
 * @signature template <typename State> Dispatcher applyMiddleware(Dispatcher baseDispatch, std::function<const State &()> getState, const std::vector<Middleware<State>> &middlewares)
 * @param baseDispatch The core store dispatch function.
 * @param getState The function returning current state.
 * @param middlewares The list of middlewares to apply.
 * @return Dispatcher The newly composed dispatch function.
 *
 * User Story: As store configuration, I need middleware composition so dispatch
 * can be enhanced without changing reducer semantics.
 */
template <typename State>
Dispatcher
applyMiddleware(Dispatcher baseDispatch,
                std::function<const State &()> getState,
                const std::vector<Middleware<State>> &middlewares) {
  /**
   * Use shared_ptr so middleware closures can reference the final enhanced
   * dispatch through indirection, matching RTK's actual behavior.
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  auto enhancedDispatch = std::make_shared<Dispatcher>(baseDispatch);

  auto api = MiddlewareApi<State>{
      /**
       * Trampoline: always calls through the fully composed chain
       * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
       */
      [enhancedDispatch](const AnyAction &action) -> AnyAction {
        return (*enhancedDispatch)(action);
      },
      getState};
  Dispatcher currentDispatch =
      detail::applyMiddlewareRecursive<State>(middlewares.rbegin(),
                                             middlewares.rend(), api,
                                             baseDispatch);

  /**
  * Point the shared dispatch at the fully composed chain
  * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
  */
  *enhancedDispatch = currentDispatch;

  return currentDispatch;
}

template <typename State> struct ListenerMiddleware {
  using EffectCallback = std::function<void(const AnyAction &action,
                                            const MiddlewareApi<State> &api)>;

  TMap<FString, TArray<EffectCallback>> listeners;
};

namespace detail {
template <typename State>
void invokeListenerEffectsRecursive(
    const TArray<typename ListenerMiddleware<State>::EffectCallback> &Effects,
    int32 Index, const AnyAction &Action, const MiddlewareApi<State> &Api) {
  Index >= Effects.Num()
      ? void()
      : (Effects[Index](Action, Api),
         invokeListenerEffectsRecursive<State>(Effects, Index + 1, Action,
                                               Api));
}

template <typename State>
void runListenerEffects(
    const TMap<FString, TArray<typename ListenerMiddleware<State>::EffectCallback>>
        &Listeners,
    const AnyAction &Action, const MiddlewareApi<State> &Api) {
  const TArray<typename ListenerMiddleware<State>::EffectCallback>
      *ActiveListeners = Listeners.Find(Action.Type);
  ActiveListeners
      ? invokeListenerEffectsRecursive<State>(*ActiveListeners, 0, Action, Api)
      : void();
}
} // namespace detail

/**
 * @brief Creates a new empty ListenerMiddleware instance.
 * @signature template <typename State> ListenerMiddleware<State> createListenerMiddleware()
 * @return ListenerMiddleware<State> The created middleware instance.
 *
 * User Story: As a functional store implementer, I need to instantiate an empty listener registry before adding side effects.
 */
template <typename State>
ListenerMiddleware<State> createListenerMiddleware() {
  return ListenerMiddleware<State>();
}

/**
 * @brief Adds an effect callback to the listener middleware for a specific action type.
 * @signature template <typename State> ListenerMiddleware<State> addListener(ListenerMiddleware<State> MiddlewareValue, const FString &ActionType, typename ListenerMiddleware<State>::EffectCallback Effect)
 * @param MiddlewareValue The middleware instance (passed by value for functional updates).
 * @param ActionType The action type string to listen for.
 * @param Effect The side effect callback to execute.
 * @return ListenerMiddleware<State> The updated middleware instance.
 *
 * User Story: As a functional store implementer, I need to immutably register side effects for dispatched actions.
 */
template <typename State>
ListenerMiddleware<State>
addListener(ListenerMiddleware<State> MiddlewareValue,
            const FString &ActionType,
            typename ListenerMiddleware<State>::EffectCallback Effect) {
  MiddlewareValue.listeners.FindOrAdd(ActionType).Add(Effect);
  return MiddlewareValue;
}

/**
 * @brief Builds the final Middleware function from the ListenerMiddleware registry.
 * @signature template <typename State> Middleware<State> buildListenerMiddleware(const ListenerMiddleware<State> &MiddlewareValue)
 * @param MiddlewareValue The configured listener middleware.
 * @return Middleware<State> A middleware function usable with applyMiddleware.
 *
 * User Story: As a functional store implementer, I need to convert my listener registry into standard composed middleware.
 */
template <typename State>
Middleware<State>
buildListenerMiddleware(const ListenerMiddleware<State> &MiddlewareValue) {
  const TMap<FString, TArray<typename ListenerMiddleware<State>::EffectCallback>>
      ListenersCopy = MiddlewareValue.listeners;
  return [ListenersCopy](const MiddlewareApi<State> &api)
             -> std::function<Dispatcher(Dispatcher)> {
    return [ListenersCopy, api](Dispatcher next) -> Dispatcher {
      return [ListenersCopy, api, next](const AnyAction &action) -> AnyAction {
        const AnyAction ResultAction = next(action);
        detail::runListenerEffects<State>(ListenersCopy, action, api);
        return ResultAction;
      };
    };
  };
}

/**
 * Phase 6: Selectors
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

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

/**
 * Phase 7: RTK Query Equivalent (API Slice)
 * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
 */

struct FApiEndpointTag {
  FString Type;
  FString Id;
};

template <typename Arg, typename Result> struct ApiEndpoint {
  FString EndpointName;
  TArray<FApiEndpointTag> ProvidesTags;
  TArray<FApiEndpointTag> InvalidatesTags;

  /**
   * Abstract request builder/executor
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  std::function<func::AsyncResult<func::HttpResult<Result>>(const Arg &)>
      RequestBuilder;
};

/**
 * Simplified dynamic slice registry mapped by string path
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename State> struct Api {
  FString ReducerPath;
  TArray<FString> TagTypes;
};

/**
 * @brief Creates an API slice registry with a defined path and tag types.
 * @signature template <typename State> Api<State> createApi(const FString &ReducerPath, const TArray<FString> &TagTypes)
 * @param ReducerPath The state path where the API slice mounts.
 * @param TagTypes The list of cache tag types supported by this API.
 * @return Api<State> The initialized API definition.
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename State>
Api<State> createApi(const FString &ReducerPath,
                               const TArray<FString> &TagTypes) {
  Api<State> Slice;
  Slice.ReducerPath = ReducerPath;
  Slice.TagTypes = TagTypes;
  return Slice;
}

/**
 * @brief Unwraps an HTTP result into an AsyncResult.
 * @signature template <typename Result> func::AsyncResult<Result> unwrapEndpointResult(func::HttpResult<Result> HttpResultValue)
 * @param HttpResultValue The HTTP result containing data or error.
 * @return func::AsyncResult<Result> The async result that resolves or rejects based on success.
 *
 * User Story: As a developer writing endpoint queries, I need standard HTTP results translated directly into chainable AsyncResults.
 */
template <typename Result>
func::AsyncResult<Result>
unwrapEndpointResult(func::HttpResult<Result> HttpResultValue) {
  return HttpResultValue.bSuccess
             ? func::createAsyncResult<Result>(
                   [HttpResultValue](std::function<void(Result)> Resolve,
                                     std::function<void(std::string)> Reject) {
                     Resolve(HttpResultValue.data);
                   })
             : func::createAsyncResult<Result>(
                   [HttpResultValue](std::function<void(Result)> Resolve,
                                     std::function<void(std::string)> Reject) {
                     Reject(HttpResultValue.error);
                   });
}

/**
 * @brief Injects a specific endpoint into an existing API slice definition.
 * @signature template <typename State, typename Arg, typename Result> AsyncThunkConfig<Result, Arg, State> injectEndpoints(const Api<State> &Slice, const ApiEndpoint<Arg, Result> &EndpointDesc)
 * @param Slice The base API slice.
 * @param EndpointDesc The endpoint descriptor.
 * @return AsyncThunkConfig<Result, Arg, State> A configured thunk managing the endpoint request lifecycle.
 *
 * User Story: As an API developer, I need to dynamically inject endpoints into a base API slice without altering the slice core.
 */
template <typename State, typename Arg, typename Result>
AsyncThunkConfig<Result, Arg, State>
injectEndpoints(const Api<State> &Slice,
               const ApiEndpoint<Arg, Result> &EndpointDesc) {
  const FString ThunkPrefix = Slice.ReducerPath + TEXT("/") + EndpointDesc.EndpointName;
  return createAsyncThunk<Result, Arg, State>(
      ThunkPrefix,
      [EndpointDesc](const Arg &arg, const ThunkApi<State> &api)
          -> func::AsyncResult<Result> {
        return func::AsyncChain::then<func::HttpResult<Result>, Result>(
            EndpointDesc.RequestBuilder(arg), unwrapEndpointResult<Result>);
      });
}

/**
 * Phase 8: Store Configuration
 * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
 */

template <typename State> struct EnhancedStore {
  std::shared_ptr<Store<State>> CoreStore;
  Dispatcher Dispatch;

  /**
   * Returns the current root-state snapshot.
   * User Story: As enhanced-store consumers, I need root-state access so
   * middleware, thunks, and selectors can inspect current runtime data.
   */
  const State &getState() const { return CoreStore->getState(); }

  /**
   * Registers a subscriber on the underlying core store.
   * User Story: As enhanced-store consumers, I need subscription support so
   * callers can react to state changes after middleware composition.
   */
  std::function<void()> subscribe(std::function<void()> Callback) {
    return CoreStore->subscribe(std::move(Callback));
  }

  /**
   * Dispatches a plain AnyAction through the enhanced middleware chain.
   * User Story: As enhanced-store consumers, I need action dispatch routed
   * through middleware so store behavior matches RTK-style semantics.
   */
  AnyAction dispatch(const AnyAction &action) const { return Dispatch(action); }

  /**
   * Dispatches a thunk using the enhanced dispatch and current getState accessors.
   * User Story: As thunk callers, I need thunk dispatch integrated with the
   * enhanced store so async flows can reuse middleware and state access.
   */
  template <typename Result>
  func::AsyncResult<Result>
  dispatch(const ThunkAction<Result, State> &thunk) const {
    auto dispatchAny = Dispatch;
    auto getState = [this]() -> const State & {
      return CoreStore->getState();
    };
    return thunk(dispatchAny, getState);
  }
};

/**
 * @brief Configures an enhanced store with middleware and preloaded state.
 * @signature template <typename State> EnhancedStore<State> configureStore(CaseReducer<State> rootReducer, State preloadedState, const std::vector<Middleware<State>> &middlewares)
 * @param rootReducer The root reducer function.
 * @param preloadedState The initial state of the store.
 * @param middlewares A vector of middlewares to apply.
 * @return EnhancedStore<State> The fully configured enhanced store.
 *
 * User Story: As runtime bootstrapping, I need a configured enhanced store so
 * reducers, middleware, and preload state come together in one helper.
 */
template <typename State>
EnhancedStore<State>
configureStore(CaseReducer<State> rootReducer, State preloadedState,
               const std::vector<Middleware<State>> &middlewares) {
  EnhancedStore<State> enhanced;
  auto coreStore = std::make_shared<Store<State>>(
      createStore(std::move(preloadedState), std::move(rootReducer)));
  enhanced.CoreStore = coreStore;

  Dispatcher coreDispatch = [coreStore](const AnyAction &action) -> AnyAction {
    return coreStore->dispatch(action);
  };

  auto getState = [coreStore]() -> const State & {
    return coreStore->getState();
  };

  enhanced.Dispatch =
      applyMiddleware<State>(coreDispatch, getState, middlewares);

  return enhanced;
}

/**
 * @brief Configures an enhanced store without middleware.
 * @signature template <typename State> EnhancedStore<State> configureStore(CaseReducer<State> rootReducer, State preloadedState)
 * @param rootReducer The root reducer function.
 * @param preloadedState The initial state of the store.
 * @return EnhancedStore<State> The configured enhanced store.
 *
 * User Story: As simple runtime bootstrapping, I need a no-middleware overload
 * so tests and small setups can create stores with less ceremony.
 */
template <typename State>
EnhancedStore<State> configureStore(CaseReducer<State> rootReducer,
                                    State preloadedState) {
  return configureStore<State>(rootReducer, preloadedState,
                               std::vector<Middleware<State>>());
}

} // namespace rtk

#endif // RTK_HPP
