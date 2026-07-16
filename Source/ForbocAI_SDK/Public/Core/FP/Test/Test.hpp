#pragma once

#include "Core/FP/Error/Error.hpp"

namespace func {
/**
 * @brief 15. TestResult (Functional Testing Result) A result type for functional testing that includes success/failure, messages, and optional detailed information. Usage: auto result = TestResult<bool>::Success(true); auto failure = TestResult<void>::Failure("Test failed");
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T> struct TestResult
 *
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */

template <typename T> struct TestResult {
  bool bSuccess;
  T value;
  std::string message;
  std::unordered_map<std::string, std::string> details;

/**
 * @fn static TestResult<T> Success(T value, std::string message = "")
 * @brief Builds a successful test result with an attached value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a success factory so assertions can
 * return values and metadata through one result type.
 */
  static TestResult<T> Success(T value, std::string message = "") {
    return TestResult<T>{true, std::move(value), std::move(message), {}};
  }

/**
 * @fn static TestResult<T> Failure(std::string message)
 * @brief Builds a failed test result with a message.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a failure factory so assertion
 * failures can be reported without exceptions or ad hoc flags.
 */
  static TestResult<T> Failure(std::string message) {
    return TestResult<T>{false, T{}, std::move(message), {}};
  }

/**
 * @fn TestResult &withDetail(const std::string &key, const std::string &val)
 * @brief Attaches a string detail pair to the result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need structured detail fields so
 * failures and successes can carry extra diagnostic context.
 */
  TestResult &withDetail(const std::string &key, const std::string &val) {
    details[key] = val;
    return *this;
  }

/**
 * @fn bool isSuccessful() const
 * @brief Reports whether the result represents success.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a direct success check so calling
 * code can branch without inspecting raw fields.
 */
  bool isSuccessful() const { return bSuccess; }

/**
 * @fn Maybe<T> TryGetValue() const
 * @brief Returns the value as a Maybe when the test succeeded.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a non-throwing accessor so
 * no-exception builds can read successful values safely.
 */
  Maybe<T> TryGetValue() const { return bSuccess ? just(value) : nothing<T>(); }

/**
 * @fn T getValue() const
 * @brief Returns the value or fails fast when the result is unsuccessful.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a strict accessor so code can
 * demand a successful value when failure is unrecoverable.
 */
  T getValue() const {
    return bSuccess ? value
                    : detail::failWithMessage<T>(
                          "TestResult: Cannot get value from failure");
  }
};

/**
 * @brief Specialization for void
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <> struct TestResult<void>
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <> struct TestResult<void> {
  bool bSuccess;
  std::string message;
  std::unordered_map<std::string, std::string> details;

/**
 * @fn static TestResult<void> Success(std::string message = "")
 * @brief Builds a successful void test result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a void success factory so
 * side-effect-only assertions can still return structured outcomes.
 */
  static TestResult<void> Success(std::string message = "") {
    return TestResult<void>{true, std::move(message), {}};
  }

/**
 * @fn static TestResult<void> Failure(std::string message)
 * @brief Builds a failed void test result with a message.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a void failure factory so
 * assertion failures can be reported even when no value is returned.
 */
  static TestResult<void> Failure(std::string message) {
    return TestResult<void>{false, std::move(message), {}};
  }

/**
 * @fn TestResult &withDetail(const std::string &key, const std::string &val)
 * @brief Attaches a string detail pair to the void result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need structured detail fields so
 * void assertions can still surface diagnostic metadata.
 */
  TestResult &withDetail(const std::string &key, const std::string &val) {
    details[key] = val;
    return *this;
  }

/**
 * @fn bool isSuccessful() const
 * @brief Reports whether the void result represents success.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional tests, I need a direct success check so callers
 * can branch on pass or fail without inspecting raw fields.
 */
  bool isSuccessful() const { return bSuccess; }
};

} // namespace func
