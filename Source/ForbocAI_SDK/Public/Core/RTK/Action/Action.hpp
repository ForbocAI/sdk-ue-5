#pragma once

#include "Core/RTK/Payload/Payload.hpp"

namespace rtk {

/**
 * 1.1 Action<Payload>
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename Payload> struct Action {
  FString Type;
  Payload PayloadValue;
};

template <typename Payload> using PayloadAction = Action<Payload>;

struct FEmptyPayload {};

namespace detail {
/**
 * @brief Helper to create a PayloadAction with a specific value.
 * @signature template <typename Payload> PayloadAction<Payload> payloadAction(const FString &Type, const Payload &PayloadValue)
 * @param Type The action type string.
 * @param PayloadValue The actual payload data.
 * @return PayloadAction<Payload> The created typed action.
 *
 * User Story: As a developer writing reducers, I need this function to easily construct strongly-typed actions with data.
 */
template <typename Payload>
PayloadAction<Payload> payloadAction(const FString &Type,
                                     const Payload &PayloadValue) {
  return PayloadAction<Payload>{Type, PayloadValue};
}

/**
 * @brief Helper to create a PayloadAction with an empty payload.
 * @signature inline PayloadAction<FEmptyPayload> payloadAction(const FString &Type)
 * @param Type The action type string.
 * @return PayloadAction<FEmptyPayload> The created empty action.
 *
 * User Story: As a developer writing reducers, I need this function to easily construct strongly-typed actions representing pure events.
 */
inline PayloadAction<FEmptyPayload> payloadAction(const FString &Type) {
  return PayloadAction<FEmptyPayload>{Type, FEmptyPayload{}};
}
} // namespace detail

/**
 * Type-erased envelope for heterogeneous root dispatch
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
struct AnyAction {
  FString Type;
  std::shared_ptr<void> PayloadWrapper;
  FString PayloadDebugText;

  /**
   * Constructs an empty type-erased action envelope.
   * User Story: As root dispatch infrastructure, I need a default AnyAction so
   * containers and return paths can be initialized before payload assignment.
   */
  AnyAction() : PayloadDebugText(TEXT("<none>")) {}

  /**
   * Constructs a type-erased action envelope from a type tag and payload wrapper.
   * User Story: As root dispatch infrastructure, I need a type-erased action
   * constructor so heterogeneous payloads can move through one dispatch channel.
   */
  AnyAction(const FString &InType, std::shared_ptr<void> InPayloadWrapper,
            const FString &InPayloadDebugText = TEXT("<opaque>"))
      : Type(InType),
        PayloadWrapper(std::move(InPayloadWrapper)),
        PayloadDebugText(InPayloadDebugText) {}

  /**
   * Extracts a typed payload from the type-erased storage.
   * User Story: As root dispatch consumers, I need a direct payload accessor so
   * infrastructure code can recover stored action data when type ownership is known.
   * Warning: This performs an unchecked static_cast. Callers must ensure the requested
   * payload type matches the stored value. Prefer ActionCreator::extract() when possible.
   */
  template <typename Payload> func::Maybe<Payload> getPayload() const {
    return PayloadWrapper
               ? func::just(*static_cast<Payload *>(PayloadWrapper.get()))
               : func::nothing<Payload>();
  }

  FString describePayload() const {
    return PayloadDebugText.IsEmpty() ? TEXT("<none>") : PayloadDebugText;
  }
};

using UnknownAction = AnyAction;

/**
 * 1.2 Reducer
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
template <typename State, typename ActionT>
using Reducer = std::function<State(const State &, const ActionT &)>;

template <typename State>
using CaseReducer = std::function<State(const State &, const AnyAction &)>;

template <typename State> struct ActionReducerMapBuilder;
template <typename Result, typename Arg, typename State> struct AsyncThunkConfig;

template <typename State> struct Store;

template <typename State>
Store<State> createStore(State InitialState, CaseReducer<State> ReducerFunc);

template <typename State> const State &getState(const Store<State> &StoreValue);

template <typename State>
AnyAction dispatch(Store<State> &StoreValue, const AnyAction &Action);

template <typename State>
std::function<void()> subscribe(Store<State> &StoreValue,
                                std::function<void()> Callback);

/**
 * 1.3 Store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
} // namespace rtk
