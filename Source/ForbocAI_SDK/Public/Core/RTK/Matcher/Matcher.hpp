#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Action/Action.hpp"

namespace rtk {
template <typename Payload> struct ActionCreator {
  FString Type;

  /** User Story: As a core rtk matcher consumer, I need to invoke the callable value through a stable signature so the core rtk matcher workflow remains explicit and composable. @fn AnyAction operator()(const Payload &payload) const */
  AnyAction operator()(const Payload &payload) const {
    return AnyAction(Type, std::make_shared<Payload>(payload),
                     payload_debug::DebugPayloadString(payload));
  }

  /**
   * Reports whether an AnyAction matches this creator's type tag.
   * User Story: As reducer helpers, I need action-type matching so handlers can
   * confirm payload shape before extraction.
   * @fn bool match(const AnyAction &action) const
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }

  /**
   * Extracts a typed payload when the action type matches this creator.
   * User Story: As reducer helpers, I need safe payload extraction so typed
   * reducers can consume AnyAction without repeating casts.
   * @fn func::Maybe<Payload> extract(const AnyAction &action) const
   */
  func::Maybe<Payload> extract(const AnyAction &action) const {
    return match(action) ? action.getPayload<Payload>()
                         : func::nothing<Payload>();
  }
};

/**
 * @fn template <typename Payload> ActionCreator<Payload> createAction(const FString &Type)
 * @brief Creates a typed action creator for a namespaced action type.
 * @param Type The action type identifier string.
 * @return ActionCreator<Payload> The action creator function object.
 *
 * User Story: As slice code, I need typed action creators so reducers and
 * thunks can share stable action contracts.
 */
template <typename Payload>
ActionCreator<Payload> createAction(const FString &Type) {
  return ActionCreator<Payload>{Type};
}

struct ActionCreatorWithoutPayload {
  FString Type;

  /** User Story: As a core rtk matcher consumer, I need to invoke the callable value through a stable signature so the core rtk matcher workflow remains explicit and composable. @fn AnyAction operator()() const */
  AnyAction operator()() const {
    return AnyAction(Type, std::make_shared<FEmptyPayload>(),
                     payload_debug::DebugPayloadString(FEmptyPayload{}));
  }

  /**
   * Reports whether an AnyAction matches this empty action creator.
   * User Story: As reducer helpers, I need empty-action matching so lifecycle
   * actions can be recognized without custom payload structs.
   * @fn bool match(const AnyAction &action) const
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }
};

/**
 * @fn inline ActionCreatorWithoutPayload createAction(const FString &Type)
 * @brief Creates an empty-payload action creator for a namespaced action type.
 * @param Type The action type identifier string.
 * @return ActionCreatorWithoutPayload The empty payload action creator object.
 *
 * User Story: As slice code, I need empty action creators so simple lifecycle
 * events can reuse the same RTK-style creation pattern.
 */
inline ActionCreatorWithoutPayload createAction(const FString &Type) {
  return ActionCreatorWithoutPayload{Type};
}


/**
 * @brief Matcher alias for RTK-style action predicates.
 * @signature using Matcher = std::function<bool(const AnyAction &)>
 *
 * User Story: As reducer and listener authors, I need a reusable action
 * predicate type so exact actions, lifecycle suffixes, and composed predicates
 * share one vocabulary.
 */
using Matcher = std::function<bool(const AnyAction &)>;

/**
 * @fn inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix)
 * @brief Checks whether an action type ends with a lifecycle suffix.
 *
 * User Story: As async reducers, I need `/pending`, `/fulfilled`, and
 * `/rejected` suffix matching to follow Redux Toolkit lifecycle semantics.
 */
inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix) {
  return Action.Type.EndsWith(Suffix);
}

/** User Story: As a core rtk matcher consumer, I need to invoke matches thunk lifecycle through a stable signature so the core rtk matcher workflow remains explicit and composable. @fn inline bool matchesThunkLifecycle(const AnyAction &Action, const FString &Suffix, const TArray<FString> &TypePrefixes) */
inline bool matchesThunkLifecycle(const AnyAction &Action, const FString &Suffix,
                                  const TArray<FString> &TypePrefixes) {
  return TypePrefixes.Num() == FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA
             ? actionTypeEndsWith(Action, Suffix)
             : TypePrefixes.ContainsByPredicate(
                   [&Action, &Suffix](const FString &TypePrefix) {
                     return Action.Type == TypePrefix + Suffix;
                   });
}

/**
 * @fn inline Matcher isAnyOf(const TArray<Matcher> &Matchers)
 * @brief Composes matchers with logical OR, equivalent to RTK isAnyOf.
 *
 * User Story: As a reducer author, I need to group several event actions under
 * one state transition without setter-style action names.
 */
inline Matcher isAnyOf(const TArray<Matcher> &Matchers) {
  return [Matchers](const AnyAction &Action) {
    return Matchers.ContainsByPredicate(
        [&Action](const Matcher &Candidate) { return Candidate(Action); });
  };
}

/**
 * @fn inline Matcher isAllOf(const TArray<Matcher> &Matchers)
 * @brief Composes matchers with logical AND, equivalent to RTK isAllOf.
 *
 * User Story: As listener middleware, I need multiple predicates to agree
 * before a side effect runs.
 */
inline Matcher isAllOf(const TArray<Matcher> &Matchers) {
  return [Matchers](const AnyAction &Action) {
    return !Matchers.ContainsByPredicate(
        [&Action](const Matcher &Candidate) { return !Candidate(Action); });
  };
}

/**
 * @fn inline Matcher isPending(const TArray<FString> &TypePrefixes = TArray<FString>())
 * @brief Creates an RTK-style matcher for `/pending` async thunk actions.
 * User Story: As a core rtk matcher consumer, I need to invoke is pending through a stable signature so the core rtk matcher workflow remains explicit and composable.
 */
inline Matcher isPending(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9BA570F18FC7), TypePrefixes);
  };
}

/**
 * @fn inline Matcher isFulfilled(const TArray<FString> &TypePrefixes = TArray<FString>())
 * @brief Creates an RTK-style matcher for `/fulfilled` async thunk actions.
 * User Story: As a core rtk matcher consumer, I need to invoke is fulfilled through a stable signature so the core rtk matcher workflow remains explicit and composable.
 */
inline Matcher isFulfilled(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0DFF16731112), TypePrefixes);
  };
}

/**
 * @fn inline Matcher isRejected(const TArray<FString> &TypePrefixes = TArray<FString>())
 * @brief Creates an RTK-style matcher for `/rejected` async thunk actions.
 * User Story: As a core rtk matcher consumer, I need to invoke is rejected through a stable signature so the core rtk matcher workflow remains explicit and composable.
 */
inline Matcher isRejected(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV626EB0E3DA73), TypePrefixes);
  };
}

/** User Story: As a core rtk matcher consumer, I need to invoke is rejected with value through a stable signature so the core rtk matcher workflow remains explicit and composable. @fn inline Matcher isRejectedWithValue(const TArray<FString> &TypePrefixes = TArray<FString>()) */
inline Matcher
isRejectedWithValue(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return Action.bRejectedWithValue &&
           matchesThunkLifecycle(Action, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV626EB0E3DA73), TypePrefixes);
  };
}

/**
 * @fn inline Matcher isAsyncThunkAction(const TArray<FString> &TypePrefixes = TArray<FString>())
 * @brief Creates an RTK-style matcher for any async thunk lifecycle action.
 * User Story: As a core rtk matcher consumer, I need to invoke is async thunk action through a stable signature so the core rtk matcher workflow remains explicit and composable.
 */
inline Matcher isAsyncThunkAction(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  TArray<Matcher> Matchers;
  Matchers.Add(isPending(TypePrefixes));
  Matchers.Add(isFulfilled(TypePrefixes));
  Matchers.Add(isRejected(TypePrefixes));
  return isAnyOf(Matchers);
}

/**
 * 2.2 Slice<State>
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
} // namespace rtk
