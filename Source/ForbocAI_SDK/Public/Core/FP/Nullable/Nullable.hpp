#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "Core/FP/Error/Error.hpp"

namespace func {
/**
 * @brief Lifts a nullable pointer into a Maybe.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> Maybe<T> from_nullable(const T *ptr)
 *
 * User Story: As boundary helpers, I need nullable pointers lifted into Maybe
 * so pointer-based APIs can join functional pipelines safely.
 */
template <typename T> Maybe<T> from_nullable(const T *ptr) {
  return ptr ? just(*ptr) : nothing<T>();
}

/**
 * @brief Lifts a value into a Maybe when the caller marks it as valid.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> Maybe<T> from_nullable_value(T value, bool valid)
 *
 * User Story: As boundary helpers, I need validity-flag lifting so non-pointer
 * APIs can still participate in Maybe-based flows.
 */
template <typename T> Maybe<T> from_nullable_value(T value, bool valid) {
  return valid ? just(std::move(value)) : nothing<T>();
}

template <typename T> Maybe<T> fromNullable(const T *ptr) {
  return from_nullable(ptr);
}

template <typename T> Maybe<T> fromNullable(T value, bool valid) {
  return from_nullable_value(std::move(value), valid);
}

/**
 * @brief Extracts a Maybe value or aborts the current boundary with an error message.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> T require_just(const Maybe<T> &m, const std::string &errorMsg)
 *
 * User Story: As boundary code, I need a fail-fast extractor so required Maybe
 * values can be enforced at integration boundaries.
 */
template <typename T>
T require_just(const Maybe<T> &m, const std::string &errorMsg) {
  return m.hasValue ? m.value : detail::failWithMessage<T>(errorMsg);
}

template <typename T>
T requireJust(const Maybe<T> &m, const std::string &errorMsg) {
  return require_just(m, errorMsg);
}

} // namespace func
