#pragma once

#include "Core/RTK/Action/Action.hpp"

namespace rtk {
template <typename Payload> struct ActionCreator {
  FString Type;

  AnyAction operator()(const Payload &payload) const {
    return AnyAction(Type, std::make_shared<Payload>(payload),
                     payload_debug::DebugPayloadString(payload));
  }

  /**
   * Reports whether an AnyAction matches this creator's type tag.
   * User Story: As reducer helpers, I need action-type matching so handlers can
   * confirm payload shape before extraction.
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }

  /**
   * Extracts a typed payload when the action type matches this creator.
   * User Story: As reducer helpers, I need safe payload extraction so typed
   * reducers can consume AnyAction without repeating casts.
   */
  func::Maybe<Payload> extract(const AnyAction &action) const {
    return match(action) ? action.getPayload<Payload>()
                         : func::nothing<Payload>();
  }
};

/**
 * @brief Creates a typed action creator for a namespaced action type.
 * @signature template <typename Payload> ActionCreator<Payload> createAction(const FString &Type)
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

  AnyAction operator()() const {
    return AnyAction(Type, std::make_shared<FEmptyPayload>(),
                     payload_debug::DebugPayloadString(FEmptyPayload{}));
  }

  /**
   * Reports whether an AnyAction matches this empty action creator.
   * User Story: As reducer helpers, I need empty-action matching so lifecycle
   * actions can be recognized without custom payload structs.
   */
  bool match(const AnyAction &action) const { return action.Type == Type; }
};

/**
 * @brief Creates an empty-payload action creator for a namespaced action type.
 * @signature inline ActionCreatorWithoutPayload createAction(const FString &Type)
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
 * @brief Checks whether an action type ends with a lifecycle suffix.
 * @signature inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix)
 *
 * User Story: As async reducers, I need `/pending`, `/fulfilled`, and
 * `/rejected` suffix matching to follow Redux Toolkit lifecycle semantics.
 */
inline bool actionTypeEndsWith(const AnyAction &Action, const FString &Suffix) {
  return Action.Type.EndsWith(Suffix);
}

inline bool matchesThunkLifecycle(const AnyAction &Action, const FString &Suffix,
                                  const TArray<FString> &TypePrefixes) {
  return TypePrefixes.Num() == 0
             ? actionTypeEndsWith(Action, Suffix)
             : TypePrefixes.ContainsByPredicate(
                   [&Action, &Suffix](const FString &TypePrefix) {
                     return Action.Type == TypePrefix + Suffix;
                   });
}

/**
 * @brief Composes matchers with logical OR, equivalent to RTK isAnyOf.
 * @signature inline Matcher isAnyOf(const TArray<Matcher> &Matchers)
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
 * @brief Composes matchers with logical AND, equivalent to RTK isAllOf.
 * @signature inline Matcher isAllOf(const TArray<Matcher> &Matchers)
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

/** @brief Creates an RTK-style matcher for `/pending` async thunk actions. */
inline Matcher isPending(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/pending"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for `/fulfilled` async thunk actions. */
inline Matcher isFulfilled(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/fulfilled"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for `/rejected` async thunk actions. */
inline Matcher isRejected(const TArray<FString> &TypePrefixes = TArray<FString>()) {
  return [TypePrefixes](const AnyAction &Action) {
    return matchesThunkLifecycle(Action, TEXT("/rejected"), TypePrefixes);
  };
}

/** @brief Creates an RTK-style matcher for any async thunk lifecycle action. */
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
