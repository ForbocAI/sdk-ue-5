#pragma once

#include "Core/RTK/Middleware/Middleware.hpp"
#include "Core/RTK/Thunk/Thunk.hpp"

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
