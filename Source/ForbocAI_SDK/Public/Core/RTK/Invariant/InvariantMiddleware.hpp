#pragma once

#include "Core/RTK/Utility/Utility.hpp"

namespace rtk {

/** User Story: As a core rtk invariant consumer, I need to invoke find non serializable value through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn inline func::Maybe<FString> findNonSerializableValue(const AnyAction &ActionValue) */
inline func::Maybe<FString>
findNonSerializableValue(const AnyAction &ActionValue) {
  return !isAction(ActionValue)
             ? func::just(ActionValue.Type)
             : (ActionValue.Meta.Contains(TEXT(""))
                    ? func::just(ActionValue.Meta.FindRef(TEXT("")))
                    : func::nothing<FString>());
}

/** User Story: As a core rtk invariant consumer, I need to invoke validates immutable state through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn template <typename State> bool validatesImmutableState( const State &StateValue, const ImmutableStateInvariantMiddlewareOptions<State> &Options) */
template <typename State>
bool validatesImmutableState(
    const State &StateValue,
    const ImmutableStateInvariantMiddlewareOptions<State> &Options) {
  return Options.IsImmutable ? Options.IsImmutable(StateValue) : true;
}

/** User Story: As a core rtk invariant consumer, I need to invoke exceeds invariant warning threshold through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn inline bool exceedsInvariantWarningThreshold(double StartedAt, float WarnAfterSeconds) */
inline bool exceedsInvariantWarningThreshold(double StartedAt,
                                             float WarnAfterSeconds) {
  return FPlatformTime::Seconds() - StartedAt > WarnAfterSeconds;
}

/** User Story: As a core rtk invariant consumer, I need to invoke create action creator invariant middleware through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn template <typename State> Middleware<State> createActionCreatorInvariantMiddleware( const ActionCreatorInvariantMiddlewareOptions &Options = ActionCreatorInvariantMiddlewareOptions()) */
template <typename State>
Middleware<State> createActionCreatorInvariantMiddleware(
    const ActionCreatorInvariantMiddlewareOptions &Options =
        ActionCreatorInvariantMiddlewareOptions()) {
  return [Options](const MiddlewareApi<State> &) {
    return [Options](Dispatcher Next) {
      return [Options, Next](const AnyAction &ActionValue) {
        const bool bIgnored =
            Options.IgnoredActionTypes.Contains(ActionValue.Type);
        (!bIgnored && !isAction(ActionValue))
            ? ensure(false)
            : true;
        return Next(ActionValue);
      };
    };
  };
}

/** User Story: As a core rtk invariant consumer, I need to invoke create immutable state invariant middleware through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn template <typename State> Middleware<State> createImmutableStateInvariantMiddleware( const ImmutableStateInvariantMiddlewareOptions<State> &Options = ImmutableStateInvariantMiddlewareOptions<State>()) */
template <typename State>
Middleware<State> createImmutableStateInvariantMiddleware(
    const ImmutableStateInvariantMiddlewareOptions<State> &Options =
        ImmutableStateInvariantMiddlewareOptions<State>()) {
  return [Options](const MiddlewareApi<State> &Api) {
    return [Options, Api](Dispatcher Next) {
      return [Options, Api, Next](const AnyAction &ActionValue) {
        const double StartedAt = FPlatformTime::Seconds();
        const bool bBeforeIsImmutable =
            validatesImmutableState(Api.getState(), Options);
        const AnyAction Result = Next(ActionValue);
        const bool bAfterIsImmutable =
            validatesImmutableState(Api.getState(), Options);
        bBeforeIsImmutable && bAfterIsImmutable
            ? true
            : ensure(false);
        exceedsInvariantWarningThreshold(StartedAt, Options.WarnAfterSeconds)
            ? ensure(false)
            : true;
        return Result;
      };
    };
  };
}

/** User Story: As a core rtk invariant consumer, I need to invoke create serializable state invariant middleware through a stable signature so the core rtk invariant workflow remains explicit and composable. @fn template <typename State> Middleware<State> createSerializableStateInvariantMiddleware( const SerializableStateInvariantMiddlewareOptions &Options = SerializableStateInvariantMiddlewareOptions()) */
template <typename State>
Middleware<State> createSerializableStateInvariantMiddleware(
    const SerializableStateInvariantMiddlewareOptions &Options =
        SerializableStateInvariantMiddlewareOptions()) {
  return [Options](const MiddlewareApi<State> &) {
    return [Options](Dispatcher Next) {
      return [Options, Next](const AnyAction &ActionValue) {
        const double StartedAt = FPlatformTime::Seconds();
        const bool bIgnored = Options.IgnoredActions.Contains(ActionValue.Type);
        const func::Maybe<FString> Invalid =
            bIgnored ? func::nothing<FString>()
                     : findNonSerializableValue(ActionValue);
        func::is_just(Invalid)
            ? ensure(false)
            : true;
        const AnyAction Result = Next(ActionValue);
        exceedsInvariantWarningThreshold(StartedAt, Options.WarnAfterSeconds)
            ? ensure(false)
            : true;
        return Result;
      };
    };
  };
}

} // namespace rtk
