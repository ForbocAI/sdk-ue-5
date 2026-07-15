#pragma once

#include "Core/rtk.hpp"
#include "Features/Protocol/Logger/LoggerAdapters.h"
#include "Features/Protocol/Logger/LoggerSelectors.h"
#include "Features/Protocol/Logger/LoggerTypes.h"

namespace LoggerListeners {

using ProtocolActionPredicate =
    std::function<bool(const rtk::AnyAction &)>;

template <typename State>
inline rtk::Middleware<State> createProtocolLoggerMiddleware(
    const FProtocolLoggerOptions &Options = FProtocolLoggerOptions(),
    const ProtocolActionPredicate &Predicate =
        LoggerSelectors::selectIsProtocolAction) {
  return [Options, Predicate](const rtk::MiddlewareApi<State> &Api)
             -> std::function<rtk::Dispatcher(rtk::Dispatcher)> {
    return [Api, Options, Predicate](rtk::Dispatcher Next)
               -> rtk::Dispatcher {
      return [Api, Next, Options, Predicate](const rtk::AnyAction &Action)
                 -> rtk::AnyAction {
        const bool bSelected = Predicate(Action);
        return !Options.bEnabled || !bSelected
                   ? Next(Action)
                   : [&]() -> rtk::AnyAction {
        const State Before = Api.getState();
        const rtk::AnyAction Result = Next(Action);
        const FString Delta =
            LoggerSelectors::describeStateDelta(Before, Api.getState());
        LoggerAdapters::writeProtocolLogAdapter(Action, Delta);
        return Result;
                     }();
      };
    };
  };
}

} // namespace LoggerListeners
