#pragma once

#include "Core/FP/Maybe/Maybe.hpp"

namespace func {
/**
 * @brief 20. multi_match (Multi-Case Value-Based Pattern Matching) Tries each predicate/handler pair in order. Returns just(handler(value)) from the first predicate that returns true. Returns nothing<R>() if no match. Helper factories: wildcard<T>()   — always-true predicate (default arm) equals<T>(val)  — value-equality predicate when<T,R>(pred, handler) — construct a MatchCase Usage: auto result = func::multi_match<FString, int>( input, { func::when<FString, int>( func::equals<FString>(TEXT("a")), [](const FString&) { return 1; }), func::when<FString, int>( func::wildcard<FString>(), [](const FString&) { return 0; }), });
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T, typename R> struct MatchCase
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T, typename R> struct MatchCase {
  std::function<bool(const T &)> predicate;
  std::function<R(const T &)> handler;
};

template <typename T> using Predicate = std::function<bool(const T &)>;

struct WildcardSentinel {};

static const WildcardSentinel _ = WildcardSentinel();

namespace detail {
/** User Story: As a core fp match consumer, I need to invoke multi match recursive through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> multiMatchRecursive(const T &Value, const std::vector<MatchCase<T, R>> &Cases, size_t Index) */
template <typename T, typename R>
Maybe<R> multiMatchRecursive(const T &Value,
                             const std::vector<MatchCase<T, R>> &Cases,
                             size_t Index) {
  return Index == Cases.size()
             ? nothing<R>()
             : (Cases[Index].predicate(Value)
                    ? just(Cases[Index].handler(Value))
                    : multiMatchRecursive<T, R>(Value, Cases, Index + 1));
}
} // namespace detail

/**
 * @fn template <typename T, typename R> MatchCase<T, R> when(std::function<bool(const T &)> pred, std::function<R(const T &)> handler)
 * @brief Builds a match case from a predicate and a handler.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As pattern-matching helpers, I need reusable cases so matching
 * logic can be declared independently from evaluation.
 */
template <typename T, typename R>
MatchCase<T, R> when(std::function<bool(const T &)> pred,
                     std::function<R(const T &)> handler) {
  MatchCase<T, R> c;
  c.predicate = std::move(pred);
  c.handler = std::move(handler);
  return c;
}

/** User Story: As a core fp match consumer, I need to invoke match case through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> MatchCase<T, R> match_case(std::function<bool(const T &)> pred, std::function<R(const T &)> handler) */
template <typename T, typename R>
MatchCase<T, R> match_case(std::function<bool(const T &)> pred,
                           std::function<R(const T &)> handler) {
  return when<T, R>(std::move(pred), std::move(handler));
}

/**
 * @fn template <typename T> std::function<bool(const T &)> wildcard()
 * @brief Returns a predicate that matches every input.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As pattern-matching helpers, I need a wildcard predicate so
 * match lists can declare explicit default branches.
 */
template <typename T> std::function<bool(const T &)> wildcard() {
  return [](const T &) { return true; };
}

/**
 * @fn template <typename T> std::function<bool(const T &)> equals(T expected)
 * @brief Returns a predicate that matches a specific expected value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As pattern-matching helpers, I need equality predicates so case
 * lists can express direct value matches declaratively.
 */
template <typename T> std::function<bool(const T &)> equals(T expected) {
  return [expected](const T &value) { return value == expected; };
}

/** User Story: As a core fp match consumer, I need to invoke test case through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> testCase(const T &value, std::function<bool(const T &)> predicate, std::function<R(const T &)> handler) */
template <typename T, typename R>
Maybe<R> testCase(const T &value, std::function<bool(const T &)> predicate,
                  std::function<R(const T &)> handler) {
  return predicate(value) ? just(handler(value)) : nothing<R>();
}

/** User Story: As a core fp match consumer, I need to invoke test case through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> testCase(const T &value, const T &expected, std::function<R(const T &)> handler) */
template <typename T, typename R>
Maybe<R> testCase(const T &value, const T &expected,
                  std::function<R(const T &)> handler) {
  return value == expected ? just(handler(value)) : nothing<R>();
}

/** User Story: As a core fp match consumer, I need to invoke test case through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> testCase(const T &value, WildcardSentinel, std::function<R(const T &)> handler) */
template <typename T, typename R>
Maybe<R> testCase(const T &value, WildcardSentinel,
                  std::function<R(const T &)> handler) {
  return just(handler(value));
}

/**
 * @fn template <typename T, typename R> Maybe<R> multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases)
 * @brief Evaluates match cases in order and returns the first successful result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As pattern-matching helpers, I need ordered case evaluation so
 * callers can express prioritized matching without manual branching.
 */
template <typename T, typename R>
Maybe<R> multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases) {
  return detail::multiMatchRecursive<T, R>(value, cases, 0);
}

/** User Story: As a core fp match consumer, I need to invoke multi match through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R, typename FWildcard> R multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases, FWildcard wildcard) */
template <typename T, typename R, typename FWildcard>
R multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases,
              FWildcard wildcard) {
  return match(multi_match<T, R>(value, cases),
               [](const R &matched) { return matched; },
               [&value, &wildcard]() { return wildcard(value); });
}

/** User Story: As a core fp match consumer, I need to invoke multi match maybe through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> multi_match_maybe(const T &value, const std::vector<MatchCase<T, R>> &cases) */
template <typename T, typename R>
Maybe<R> multi_match_maybe(const T &value,
                           const std::vector<MatchCase<T, R>> &cases) {
  return multi_match<T, R>(value, cases);
}

/** User Story: As a core fp match consumer, I need to invoke multi match through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R> Maybe<R> multiMatch(const T &value, const std::vector<MatchCase<T, R>> &cases) */
template <typename T, typename R>
Maybe<R> multiMatch(const T &value,
                    const std::vector<MatchCase<T, R>> &cases) {
  return multi_match<T, R>(value, cases);
}

/** User Story: As a core fp match consumer, I need to invoke multi match through a stable signature so the core fp match workflow remains explicit and composable. @fn template <typename T, typename R, typename FWildcard> R multiMatch(const T &value, const std::vector<MatchCase<T, R>> &cases, FWildcard wildcard) */
template <typename T, typename R, typename FWildcard>
R multiMatch(const T &value, const std::vector<MatchCase<T, R>> &cases,
             FWildcard wildcard) {
  return multi_match<T, R>(value, cases, wildcard);
}

} // namespace func
