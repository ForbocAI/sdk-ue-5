#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Action/Action.hpp"

namespace rtk {
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
/** User Story: As a core rtk middleware consumer, I need to invoke apply middleware recursive through a stable signature so the core rtk middleware workflow remains explicit and composable. @fn template <typename State> Dispatcher applyMiddlewareRecursive( typename std::vector<Middleware<State>>::const_reverse_iterator It, typename std::vector<Middleware<State>>::const_reverse_iterator End, const MiddlewareApi<State> &Api, Dispatcher CurrentDispatch) */
template <typename State>
Dispatcher applyMiddlewareRecursive(
    typename std::vector<Middleware<State>>::const_reverse_iterator It,
    typename std::vector<Middleware<State>>::const_reverse_iterator End,
    const MiddlewareApi<State> &Api, Dispatcher CurrentDispatch) {
  return It == End
             ? CurrentDispatch
             : applyMiddlewareRecursive<State>(It + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, End, Api,
                                               (*It)(Api)(CurrentDispatch));
}
} // namespace detail

/**
 * @fn template <typename State> Dispatcher applyMiddleware(Dispatcher baseDispatch, std::function<const State &()> getState, const std::vector<Middleware<State>> &middlewares)
 * @brief Wraps a dispatcher with middleware while preserving RTK-style composition order.
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

} // namespace rtk
