#pragma once

#include "Core/RTK/Types/Types.hpp"

namespace rtk {

template <typename State> struct DynamicMiddlewareInstance {
  struct Registry {
    std::vector<Middleware<State>> MiddlewareValues;
  };

  std::shared_ptr<Registry> MiddlewareRegistry = std::make_shared<Registry>();

  /** User Story: As a core rtk dynamic consumer, I need to invoke add middleware through a stable signature so the core rtk dynamic workflow remains explicit and composable. @fn void addMiddleware(const Middleware<State> &Value) */
  void addMiddleware(const Middleware<State> &Value) {
    MiddlewareRegistry->MiddlewareValues.push_back(Value);
  }

  /** User Story: As a core rtk dynamic consumer, I need to invoke middleware through a stable signature so the core rtk dynamic workflow remains explicit and composable. @fn Middleware<State> middleware() const */
  Middleware<State> middleware() const {
    const std::shared_ptr<Registry> RegistryValue = MiddlewareRegistry;
    return [RegistryValue](const MiddlewareApi<State> &Api) {
      return [RegistryValue, Api](Dispatcher Next) {
        return [RegistryValue, Api, Next](const AnyAction &ActionValue) {
          const std::vector<Middleware<State>> Snapshot =
              RegistryValue->MiddlewareValues;
          const Dispatcher DynamicDispatch =
              detail::applyMiddlewareRecursive<State>(
                  Snapshot.rbegin(), Snapshot.rend(), Api, Next);
          return DynamicDispatch(ActionValue);
        };
      };
    };
  }
};

/** User Story: As a core rtk dynamic consumer, I need to invoke create dynamic middleware through a stable signature so the core rtk dynamic workflow remains explicit and composable. @fn template <typename State> DynamicMiddlewareInstance<State> createDynamicMiddleware() */
template <typename State>
DynamicMiddlewareInstance<State> createDynamicMiddleware() {
  return DynamicMiddlewareInstance<State>();
}

} // namespace rtk
