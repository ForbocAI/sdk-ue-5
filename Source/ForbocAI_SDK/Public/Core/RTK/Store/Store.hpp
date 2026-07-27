#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Action/Action.hpp"

namespace rtk {
template <typename State> struct Store {
  State CurrentState;
  CaseReducer<State> RootReducer;

  struct Subscriber {
    int64_t Id;
    std::function<void()> Callback;
  };
  std::vector<Subscriber> Subscribers;
  int64_t NextId = FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;

  /**
   * Returns the current store state snapshot.
   * User Story: As store consumers, I need the current state exposed so
   * dispatchers, selectors, and subscribers can inspect runtime data.
   * @fn const State &getState() const
   */
  const State &getState() const { return rtk::getState(*this); }

  /**
   * Applies an action and notifies subscribers after the reducer runs.
   * User Story: As store consumers, I need dispatch to update state and notify
   * listeners so runtime flows can react to reducer changes.
   * @fn AnyAction dispatch(const AnyAction &action)
   */
  AnyAction dispatch(const AnyAction &action) {
    return rtk::dispatch(*this, action);
  }

  /**
   * Registers a callback and returns an unsubscribe function.
   * User Story: As store consumers, I need subscriptions with unsubscribe
   * handles so runtime code can observe state safely.
   * @fn std::function<void()> subscribe(std::function<void()> Callback)
   */
  std::function<void()> subscribe(std::function<void()> Callback) {
    return rtk::subscribe(*this, std::move(Callback));
  }
};

namespace detail {
/**
 * @fn template <typename State> void notifySubscribersRecursive( const std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index)
 * @brief Recursively notifies all subscribers in the list.
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
         notifySubscribersRecursive<State>(Subscribers, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4));
}

/**
 * @fn template <typename State> void eraseSubscriberAt(std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index)
 * @brief Erases a subscriber at a specific index.
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
 * @fn template <typename State> void eraseSubscriberRecursive( std::vector<typename Store<State>::Subscriber> &Subscribers, size_t Index, int64_t Id)
 * @brief Recursively finds and removes a subscriber by Id.
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
             : eraseSubscriberRecursive<State>(Subscribers, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, Id),
         void());
}

/** User Story: As a core rtk store consumer, I need to invoke reduce store through a stable signature so the core rtk store workflow remains explicit and composable. @fn template <typename State> void reduceStore(Store<State> &StoreValue, const AnyAction &Action) */
template <typename State>
void reduceStore(Store<State> &StoreValue, const AnyAction &Action) {
  StoreValue.CurrentState =
      StoreValue.RootReducer(StoreValue.CurrentState, Action);
}

/** User Story: As a core rtk store consumer, I need to invoke notify store subscribers through a stable signature so the core rtk store workflow remains explicit and composable. @fn template <typename State> void notifyStoreSubscribers(Store<State> &StoreValue) */
template <typename State>
void notifyStoreSubscribers(Store<State> &StoreValue) {
  const std::vector<typename Store<State>::Subscriber> Subscribers =
      StoreValue.Subscribers;
  notifySubscribersRecursive<State>(Subscribers, size_t{});
}
} // namespace detail

/**
 * @fn template <typename State> Store<State> createStore(State InitialState, CaseReducer<State> ReducerFunc)
 * @brief Creates a Redux-like store with an initial state and root reducer.
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

/** User Story: As a core rtk store consumer, I need to invoke legacy create store through a stable signature so the core rtk store workflow remains explicit and composable. @fn template <typename State> Store<State> legacy_createStore(State InitialState, CaseReducer<State> ReducerFunc) */
template <typename State>
Store<State> legacy_createStore(State InitialState,
                                CaseReducer<State> ReducerFunc) {
  return createStore(MoveTemp(InitialState), MoveTemp(ReducerFunc));
}

/**
 * @fn template <typename State> const State &getState(const Store<State> &StoreValue)
 * @brief Gets the current state from a given store.
 * @param StoreValue The store to query.
 * @return const State& A const reference to the current state.
 *
 * User Story: As a developer, I need this to read the current state synchronously from a store instance.
 */
template <typename State> const State &getState(const Store<State> &StoreValue) {
  return StoreValue.CurrentState;
}

/**
 * @fn template <typename State> AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action)
 * @brief Dispatches an action to a store, updating its state and notifying subscribers.
 * @param StoreValue The store to dispatch to.
 * @param Action The type-erased action to process.
 * @return AnyAction The dispatched action.
 *
 * User Story: As a developer, I need this function to trigger state updates predictably.
 */
template <typename State>
AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action) {
  detail::reduceStore(StoreValue, Action);
  detail::notifyStoreSubscribers(StoreValue);
  return Action;
}

/**
 * @fn template <typename State> std::function<void()> subscribe(Store<State> &StoreValue, std::function<void()> Callback)
 * @brief Subscribes a callback to state changes in the store.
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
    detail::eraseSubscriberRecursive<State>(StoreValue.Subscribers, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, Id);
  };
}

/**
 * 1.4 combineReducers
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
} // namespace rtk
