#pragma once

#include "Core/RTK/Matcher/Matcher.hpp"

namespace rtk {

template <typename Payload>
using ActionCreatorWithPayload = ActionCreator<Payload>;

template <typename Payload>
using PayloadActionCreator = ActionCreatorWithPayload<Payload>;

template <typename Payload> struct ActionCreatorWithOptionalPayload {
  FString Type;

  /** User Story: As a core rtk action consumer, I need to invoke the callable value through a stable signature so the core rtk action workflow remains explicit and composable. @fn AnyAction operator()(const Payload &Value) const */
  AnyAction operator()(const Payload &Value) const {
    return ActionCreator<Payload>{Type}(Value);
  }

  /** User Story: As a core rtk action consumer, I need to invoke the callable value through a stable signature so the core rtk action workflow remains explicit and composable. @fn AnyAction operator()() const */
  AnyAction operator()() const {
    AnyAction Result;
    Result.Type = Type;
    return Result;
  }

  /** User Story: As a core rtk action consumer, I need to invoke match through a stable signature so the core rtk action workflow remains explicit and composable. @fn bool match(const AnyAction &Value) const */
  bool match(const AnyAction &Value) const { return Value.Type == Type; }
};

template <typename Payload>
using ActionCreatorWithNonInferrablePayload =
    ActionCreatorWithOptionalPayload<Payload>;

template <typename Payload> struct PreparedAction {
  Payload PayloadValue;
  TMap<FString, FString> Meta;
  bool bError = false;
};

template <typename Payload, typename... Args>
using PrepareAction = std::function<PreparedAction<Payload>(Args...)>;

template <typename Payload, typename... Args>
struct ActionCreatorWithPreparedPayload {
  FString Type;
  PrepareAction<Payload, Args...> Prepare;

  /** User Story: As a core rtk action consumer, I need to invoke the callable value through a stable signature so the core rtk action workflow remains explicit and composable. @fn AnyAction operator()(Args... Values) const */
  AnyAction operator()(Args... Values) const {
    const PreparedAction<Payload> Prepared = Prepare(Values...);
    AnyAction Result = ActionCreator<Payload>{Type}(Prepared.PayloadValue);
    Result.Meta = Prepared.Meta;
    Result.bError = Prepared.bError;
    return Result;
  }

  /** User Story: As a core rtk action consumer, I need to invoke match through a stable signature so the core rtk action workflow remains explicit and composable. @fn bool match(const AnyAction &Value) const */
  bool match(const AnyAction &Value) const { return Value.Type == Type; }
};

using ActionMatchingAllOf = AnyAction;
using ActionMatchingAnyOf = AnyAction;
using Actions = TArray<AnyAction>;

/** User Story: As a core rtk action consumer, I need to invoke is action through a stable signature so the core rtk action workflow remains explicit and composable. @fn inline bool isAction(const AnyAction &Value) */
inline bool isAction(const AnyAction &Value) { return !Value.Type.IsEmpty(); }

/** User Story: As a core rtk action consumer, I need to invoke is action creator through a stable signature so the core rtk action workflow remains explicit and composable. @fn template <typename Payload> bool isActionCreator(const ActionCreator<Payload> &Value) */
template <typename Payload>
bool isActionCreator(const ActionCreator<Payload> &Value) {
  return !Value.Type.IsEmpty();
}

/** User Story: As a core rtk action consumer, I need to invoke is action creator through a stable signature so the core rtk action workflow remains explicit and composable. @fn inline bool isActionCreator(const ActionCreatorWithoutPayload &Value) */
inline bool isActionCreator(const ActionCreatorWithoutPayload &Value) {
  return !Value.Type.IsEmpty();
}

/** User Story: As a core rtk action consumer, I need to invoke is flux standard action through a stable signature so the core rtk action workflow remains explicit and composable. @fn inline bool isFluxStandardAction(const AnyAction &Value) */
inline bool isFluxStandardAction(const AnyAction &Value) {
  return isAction(Value);
}

/** User Story: As a core rtk action consumer, I need to invoke bind action creators through a stable signature so the core rtk action workflow remains explicit and composable. @fn template <typename Payload> std::function<AnyAction(const Payload &)> bindActionCreators(const ActionCreator<Payload> &Creator, std::function<AnyAction(const AnyAction &)> DispatchValue) */
template <typename Payload>
std::function<AnyAction(const Payload &)>
bindActionCreators(const ActionCreator<Payload> &Creator,
                   std::function<AnyAction(const AnyAction &)> DispatchValue) {
  return [Creator, DispatchValue](const Payload &Value) {
    return DispatchValue(Creator(Value));
  };
}

/** User Story: As a core rtk action consumer, I need to invoke bind action creators through a stable signature so the core rtk action workflow remains explicit and composable. @fn inline std::function<AnyAction()> bindActionCreators(const ActionCreatorWithoutPayload &Creator, std::function<AnyAction(const AnyAction &)> DispatchValue) */
inline std::function<AnyAction()>
bindActionCreators(const ActionCreatorWithoutPayload &Creator,
                   std::function<AnyAction(const AnyAction &)> DispatchValue) {
  return [Creator, DispatchValue]() { return DispatchValue(Creator()); };
}

} // namespace rtk
