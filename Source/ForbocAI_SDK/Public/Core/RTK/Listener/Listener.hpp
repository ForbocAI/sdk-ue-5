#pragma once

#include "Core/RTK/Middleware/Middleware.hpp"

namespace rtk {
template <typename State> struct ListenerMiddleware {
  using EffectCallback = std::function<void(const AnyAction &action,
                                            const MiddlewareApi<State> &api)>;

  TMap<FString, TArray<EffectCallback>> listeners;
};

namespace detail {
/** User Story: As a core rtk listener consumer, I need to invoke invoke listener effects recursive through a stable signature so the core rtk listener workflow remains explicit and composable. @fn template <typename State> void invokeListenerEffectsRecursive( const TArray<typename ListenerMiddleware<State>::EffectCallback> &Effects, int32 Index, const AnyAction &Action, const MiddlewareApi<State> &Api) */
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

/** User Story: As a core rtk listener consumer, I need to invoke run listener effects through a stable signature so the core rtk listener workflow remains explicit and composable. @fn template <typename State> void runListenerEffects( const TMap<FString, TArray<typename ListenerMiddleware<State>::EffectCallback>> &Listeners, const AnyAction &Action, const MiddlewareApi<State> &Api) */
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
 * @fn template <typename State> ListenerMiddleware<State> createListenerMiddleware()
 * @brief Creates a new empty ListenerMiddleware instance.
 * @return ListenerMiddleware<State> The created middleware instance.
 *
 * User Story: As a functional store implementer, I need to instantiate an empty listener registry before adding side effects.
 */
template <typename State>
ListenerMiddleware<State> createListenerMiddleware() {
  return ListenerMiddleware<State>();
}

/**
 * @fn template <typename State> ListenerMiddleware<State> addListener(ListenerMiddleware<State> MiddlewareValue, const FString &ActionType, typename ListenerMiddleware<State>::EffectCallback Effect)
 * @brief Adds an effect callback to the listener middleware for a specific action type.
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

/** User Story: As a core rtk listener consumer, I need to invoke remove listener through a stable signature so the core rtk listener workflow remains explicit and composable. @fn template <typename State> ListenerMiddleware<State> removeListener(ListenerMiddleware<State> MiddlewareValue, const FString &ActionType) */
template <typename State>
ListenerMiddleware<State>
removeListener(ListenerMiddleware<State> MiddlewareValue,
               const FString &ActionType) {
  MiddlewareValue.listeners.Remove(ActionType);
  return MiddlewareValue;
}

/** User Story: As a core rtk listener consumer, I need to invoke clear all listeners through a stable signature so the core rtk listener workflow remains explicit and composable. @fn template <typename State> ListenerMiddleware<State> clearAllListeners(ListenerMiddleware<State> MiddlewareValue) */
template <typename State>
ListenerMiddleware<State>
clearAllListeners(ListenerMiddleware<State> MiddlewareValue) {
  MiddlewareValue.listeners.Empty();
  return MiddlewareValue;
}

/**
 * @fn template <typename State> Middleware<State> buildListenerMiddleware(const ListenerMiddleware<State> &MiddlewareValue)
 * @brief Builds the final Middleware function from the ListenerMiddleware registry.
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

} // namespace rtk
