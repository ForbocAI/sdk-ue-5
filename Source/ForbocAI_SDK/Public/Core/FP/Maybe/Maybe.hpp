#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
/**
 * @brief 3. DATA: Maybe (Optional Monad) A value that may or may not exist. Pure data struct — no methods. Construction: use factory functions just() / nothing() Operations:   use free functions fmap() / mbind() / or_else() Requires: T is default-constructible.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> struct Maybe
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct Maybe {
  bool hasValue;
  T value;
};

/**
 * @fn template <typename T> Maybe<T> just(T v)
 * @brief Wraps a concrete value in a populated Maybe.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As optional flows, I need a simple way to lift a value into
 * Maybe so absence and presence stay explicit in pipelines.
 */
template <typename T> Maybe<T> just(T v) {
  return Maybe<T>{true, std::move(v)};
}

/**
 * @fn template <typename T> Maybe<T> nothing()
 * @brief Builds an empty Maybe with a default-constructed payload.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As optional flows, I need a canonical empty Maybe so code can
 * represent missing values without custom sentinels.
 */
template <typename T> Maybe<T> nothing() {
  static_assert(
      std::is_default_constructible<T>::value,
      "func::nothing<T>() requires a default-constructible payload type");
  return Maybe<T>{false, T{}};
}

/**
 * @fn template <typename T> bool is_just(const Maybe<T> &m)
 * @brief Returns true when a Maybe contains a value.
 *
 *
 * User Story: As a cross-SDK caller, I need an explicit presence predicate so
 * optional flows read the same way in C++, Rust, TypeScript, and GDScript.
 */
template <typename T> bool is_just(const Maybe<T> &m) { return m.hasValue; }

/**
 * @fn template <typename T> bool is_nothing(const Maybe<T> &m)
 * @brief Returns true when a Maybe is empty.
 *
 *
 * User Story: As a boundary author, I need an explicit absence predicate so
 * absence branches are visible instead of hidden behind raw struct fields.
 */
template <typename T> bool is_nothing(const Maybe<T> &m) {
  return !m.hasValue;
}

/** User Story: As a core fp maybe consumer, I need to invoke is just through a stable signature so the core fp maybe workflow remains explicit and composable. @fn template <typename T> bool isJust(const Maybe<T> &m) */
template <typename T> bool isJust(const Maybe<T> &m) { return is_just(m); }

/** User Story: As a core fp maybe consumer, I need to invoke is nothing through a stable signature so the core fp maybe workflow remains explicit and composable. @fn template <typename T> bool isNothing(const Maybe<T> &m) */
template <typename T> bool isNothing(const Maybe<T> &m) {
  return is_nothing(m);
}

/**
 * @fn template <typename T, typename Predicate> Maybe<T> maybe_filter(const Maybe<T> &m, Predicate predicate)
 * @brief Keeps a Maybe value only when it satisfies a predicate.
 *
 *
 * User Story: As a validation author, I need to refine a present value into
 * Nothing when it fails a rule so the rest of the pipeline short-circuits.
 */
template <typename T, typename Predicate>
Maybe<T> maybe_filter(const Maybe<T> &m, Predicate predicate) {
  std::function<Maybe<T>()> PresenceCases[2] = {
      []() { return nothing<T>(); },
      [&]() {
        std::function<Maybe<T>()> PredicateCases[2] = {
            []() { return nothing<T>(); }, [&]() { return m; }};
        return PredicateCases[static_cast<size_t>(predicate(m.value))]();
      }};
  return PresenceCases[static_cast<size_t>(m.hasValue)]();
}

/**
 * @fn template <typename T> Maybe<T> maybe_from_option(Maybe<T> opt)
 * @brief Converts a host Maybe-shaped value into the functional Maybe type.
 *
 *
 * User Story: As an interop author, I need a named bridge that mirrors Rust
 * Option conversion even though C++11 uses the same Maybe data shape here.
 */
template <typename T> Maybe<T> maybe_from_option(Maybe<T> opt) {
  return opt;
}

/**
 * @fn template <typename T> Maybe<T> maybe_to_option(Maybe<T> maybe)
 * @brief Converts a functional Maybe back to the host option shape.
 *
 *
 * User Story: As an SDK maintainer, I need a symmetrical lowering helper so
 * examples and generated docs can use the same names across languages.
 */
template <typename T> Maybe<T> maybe_to_option(Maybe<T> maybe) {
  return maybe;
}

} // namespace func
