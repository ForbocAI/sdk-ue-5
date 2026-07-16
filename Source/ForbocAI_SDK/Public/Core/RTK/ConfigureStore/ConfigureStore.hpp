#pragma once

#include "Async/Async.h"
#include "Containers/Ticker.h"
#include "Core/RTK/Batch/AutoBatchTypes.hpp"
#include "Core/RTK/Middleware/Middleware.hpp"
#include "Core/RTK/Thunk/Thunk.hpp"

#include <atomic>

namespace rtk {
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
   * @fn const State &getState() const
   */
  const State &getState() const { return CoreStore->getState(); }

  /**
   * Registers a subscriber on the underlying core store.
   * User Story: As enhanced-store consumers, I need subscription support so
   * callers can react to state changes after middleware composition.
   * @fn std::function<void()> subscribe(std::function<void()> Callback)
   */
  std::function<void()> subscribe(std::function<void()> Callback) {
    return CoreStore->subscribe(std::move(Callback));
  }

  /**
   * Dispatches a plain AnyAction through the enhanced middleware chain.
   * User Story: As enhanced-store consumers, I need action dispatch routed
   * through middleware so store behavior matches RTK-style semantics.
   * @fn AnyAction dispatch(const AnyAction &action) const
   */
  AnyAction dispatch(const AnyAction &action) const { return Dispatch(action); }

  /**
   * Dispatches a thunk using the enhanced dispatch and current getState accessors.
   * User Story: As thunk callers, I need thunk dispatch integrated with the
   * enhanced store so async flows can reuse middleware and state access.
   * @fn template <typename Result> func::AsyncResult<Result> dispatch(const ThunkAction<Result, State> &thunk) const
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

namespace detail {
/** User Story: As a core rtk configure store consumer, I need to invoke queue auto batch on game thread through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn inline void queueAutoBatchOnGameThread(std::function<void()> Notification) */
inline void queueAutoBatchOnGameThread(std::function<void()> Notification) {
  AsyncTask(ENamedThreads::GameThread, std::move(Notification));
}

/** User Story: As a core rtk configure store consumer, I need to invoke queue auto batch on ticker through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn inline void queueAutoBatchOnTicker(float DelaySeconds, std::function<void()> Notification) */
inline void queueAutoBatchOnTicker(float DelaySeconds,
                                   std::function<void()> Notification) {
  FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateLambda(
          [Notification](float) {
            Notification();
            return false;
          }),
      FMath::Max(DelaySeconds, float{}));
}

/** User Story: As a core rtk configure store consumer, I need to invoke default auto batch queue through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn inline std::function<void(std::function<void()>)> defaultAutoBatchQueue(const AutoBatchOptions &Options) */
inline std::function<void(std::function<void()>)>
defaultAutoBatchQueue(const AutoBatchOptions &Options) {
  return Options.Schedule == AutoBatchOptions::Type::Timer
             ? std::function<void(std::function<void()>)>(
                   [Options](std::function<void()> Notification) {
                     queueAutoBatchOnTicker(Options.TimeoutSeconds,
                                            std::move(Notification));
                   })
             : std::function<void(std::function<void()>)>(
                   [](std::function<void()> Notification) {
                     queueAutoBatchOnGameThread(std::move(Notification));
                   });
}

/** User Story: As a core rtk configure store consumer, I need to invoke resolve auto batch queue through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn inline std::function<void(std::function<void()>)> resolveAutoBatchQueue(const AutoBatchOptions &Options) */
inline std::function<void(std::function<void()>)>
resolveAutoBatchQueue(const AutoBatchOptions &Options) {
  return Options.QueueNotification ? Options.QueueNotification
                                   : defaultAutoBatchQueue(Options);
}

template <typename State> struct AutoBatchNotificationState {
  std::weak_ptr<Store<State>> StoreValue;
  std::atomic<bool> bPending{false};
};

/** User Story: As a core rtk configure store consumer, I need to invoke flush auto batch notification through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn template <typename State> void flushAutoBatchNotification( const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState) */
template <typename State>
void flushAutoBatchNotification(
    const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState) {
  const std::shared_ptr<Store<State>> StoreValue =
      NotificationState->StoreValue.lock();
  const bool bShouldNotify = NotificationState->bPending.exchange(false);
  bShouldNotify && StoreValue
      ? notifyStoreSubscribers(*StoreValue)
      : void();
}

/** User Story: As a core rtk configure store consumer, I need to invoke queue auto batch notification through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn template <typename State> void queueAutoBatchNotification( const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState, const std::function<void(std::function<void()>)> &QueueNotification) */
template <typename State>
void queueAutoBatchNotification(
    const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState,
    const std::function<void(std::function<void()>)> &QueueNotification) {
  const bool bAlreadyPending = NotificationState->bPending.exchange(true);
  bAlreadyPending
      ? void()
      : QueueNotification([NotificationState]() {
          flushAutoBatchNotification(NotificationState);
        });
}

/** User Story: As a core rtk configure store consumer, I need to invoke notify unbatched action through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn template <typename State> void notifyUnbatchedAction( const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState, Store<State> &StoreValue) */
template <typename State>
void notifyUnbatchedAction(
    const std::shared_ptr<AutoBatchNotificationState<State>> &NotificationState,
    Store<State> &StoreValue) {
  NotificationState->bPending.exchange(false);
  notifyStoreSubscribers(StoreValue);
}

/** User Story: As a core rtk configure store consumer, I need to invoke create auto batched dispatch through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn template <typename State> Dispatcher createAutoBatchedDispatch( const std::shared_ptr<Store<State>> &StoreValue, const AutoBatchOptions &Options) */
template <typename State>
Dispatcher createAutoBatchedDispatch(
    const std::shared_ptr<Store<State>> &StoreValue,
    const AutoBatchOptions &Options) {
  const std::shared_ptr<AutoBatchNotificationState<State>> NotificationState =
      std::make_shared<AutoBatchNotificationState<State>>();
  NotificationState->StoreValue = StoreValue;
  const std::function<void(std::function<void()>)> QueueNotification =
      resolveAutoBatchQueue(Options);
  return [StoreValue, NotificationState,
          QueueNotification](const AnyAction &ActionValue) {
    reduceStore(*StoreValue, ActionValue);
    ActionValue.bAutoBatch
        ? queueAutoBatchNotification(NotificationState, QueueNotification)
        : notifyUnbatchedAction(NotificationState, *StoreValue);
    return ActionValue;
  };
}
} // namespace detail

/**
 * @fn template <typename State> EnhancedStore<State> configureStore(CaseReducer<State> rootReducer, State preloadedState, const std::vector<Middleware<State>> &middlewares, const AutoBatchEnhancer &BatchEnhancer)
 * @brief Configures an enhanced store with middleware and preloaded state.
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
               const std::vector<Middleware<State>> &middlewares,
               const AutoBatchEnhancer &BatchEnhancer) {
  EnhancedStore<State> enhanced;
  auto coreStore = std::make_shared<Store<State>>(
      createStore(std::move(preloadedState), std::move(rootReducer)));
  enhanced.CoreStore = coreStore;

  Dispatcher coreDispatch =
      detail::createAutoBatchedDispatch(coreStore, BatchEnhancer.Options);

  auto getState = [coreStore]() -> const State & {
    return coreStore->getState();
  };

  enhanced.Dispatch =
      applyMiddleware<State>(coreDispatch, getState, middlewares);

  return enhanced;
}

/** User Story: As a core rtk configure store consumer, I need to invoke configure store through a stable signature so the core rtk configure store workflow remains explicit and composable. @fn template <typename State> EnhancedStore<State> configureStore(CaseReducer<State> rootReducer, State preloadedState, const std::vector<Middleware<State>> &middlewares) */
template <typename State>
EnhancedStore<State>
configureStore(CaseReducer<State> rootReducer, State preloadedState,
               const std::vector<Middleware<State>> &middlewares) {
  return configureStore<State>(
      std::move(rootReducer), std::move(preloadedState), middlewares,
      AutoBatchEnhancer{AutoBatchOptions()});
}

/**
 * @fn template <typename State> EnhancedStore<State> configureStore(CaseReducer<State> rootReducer, State preloadedState)
 * @brief Configures an enhanced store without middleware.
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
