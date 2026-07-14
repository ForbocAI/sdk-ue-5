#pragma once

#include "Core/rtk.hpp"
#include "Features/Logging/LoggingSelectors.h"
#include "Features/Logging/LoggingTypes.h"

namespace LoggingListeners {

template <typename State>
inline rtk::Middleware<State> createProtocolLoggerMiddleware() {
  return [](const rtk::MiddlewareApi<State> &Api)
             -> std::function<rtk::Dispatcher(rtk::Dispatcher)> {
    return [Api](rtk::Dispatcher Next) -> rtk::Dispatcher {
      return [Api, Next](const rtk::AnyAction &Action) -> rtk::AnyAction {
        const State Before = Api.getState();
        const rtk::AnyAction Result = Next(Action);
        const FString Delta =
            LoggingSelectors::describeStateDelta(Before, Api.getState());
        UE_LOG(LogForbocAIProtocol, Display,
               TEXT("[ForbocAI][Protocol] action=%s payload=%s delta=%s"),
               *Action.Type, *Action.describePayload(), *Delta);
        return Result;
      };
    };
  };
}

} // namespace LoggingListeners
