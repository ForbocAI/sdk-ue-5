#pragma once

#include "Core/FP/Either/Either.hpp"

namespace func {
/**
 * @brief 13. ValidationPipeline (Functional Validation Chain) A pipeline for chaining validation functions. Each validation function takes input and returns Either<Error, Result>. The pipeline short-circuits on first error. Usage: auto pipeline = validationPipeline<int>() | validatePositive | validateRange | validateEven; auto result = runValidation(pipeline, 42);
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename T, typename E = std::string> struct ValidationPipeline
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T, typename E = std::string> struct ValidationPipeline {
  std::vector<std::function<Either<E, T>(T)>> Validators;
};

/**
 * @fn template <typename T, typename E = std::string> ValidationPipeline<T, E> validationPipeline()
 * @brief Creates an empty validation pipeline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As validation flows, I need a pipeline entry point so validators
 * can be declared and composed incrementally.
 */
template <typename T, typename E = std::string>
ValidationPipeline<T, E> validationPipeline() {
  return ValidationPipeline<T, E>{{}};
}

/**
 * @fn template <typename T, typename E, typename Func> ValidationPipeline<T, E> addValidation(ValidationPipeline<T, E> Pipeline, Func Validator)
 * @brief Appends a validator to the pipeline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As validation flows, I need validators chained fluently so input
 * rules can be assembled without stateful builder objects.
 */
template <typename T, typename E, typename Func>
ValidationPipeline<T, E> addValidation(ValidationPipeline<T, E> Pipeline,
                                       Func Validator) {
  std::function<Either<E, T>(T)> WrappedValidator = Validator;
  Pipeline.Validators.push_back(WrappedValidator);
  return Pipeline;
}

/**
 * @fn template <typename T, typename E, typename Func> ValidationPipeline<T, E> operator|(ValidationPipeline<T, E> Pipeline, Func Validator)
 * @brief Supports pipe-style validation assembly with free functions.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As validation flows, I need ergonomic composition so validators
 * can still be chained declaratively after removing member builders.
 */
template <typename T, typename E, typename Func>
ValidationPipeline<T, E> operator|(ValidationPipeline<T, E> Pipeline,
                                   Func Validator) {
  return addValidation(std::move(Pipeline), Validator);
}

namespace detail {
/** User Story: As a core fp validation consumer, I need to invoke run validation recursive through a stable signature so the core fp validation workflow remains explicit and composable. @fn template <typename T, typename E> Either<E, T> runValidationRecursive(const std::vector<std::function<Either<E, T>(T)>> &Steps, size_t Index, T Current) */
template <typename T, typename E>
Either<E, T>
runValidationRecursive(const std::vector<std::function<Either<E, T>(T)>> &Steps,
                       size_t Index, T Current);

/** User Story: As a core fp validation consumer, I need to invoke run validation step through a stable signature so the core fp validation workflow remains explicit and composable. @fn template <typename T, typename E> Either<E, T> runValidationStep(const std::vector<std::function<Either<E, T>(T)>> &Steps, size_t Index, T Current) */
template <typename T, typename E>
Either<E, T>
runValidationStep(const std::vector<std::function<Either<E, T>(T)>> &Steps,
                  size_t Index, T Current) {
  Either<E, T> Result = Steps[Index](Current);
  return Result.isLeft ? Result
                       : runValidationRecursive<T, E>(Steps, Index + 1,
                                                      Result.right);
}

/** User Story: As a core fp validation consumer, I need to invoke run validation recursive through a stable signature so the core fp validation workflow remains explicit and composable. @fn template <typename T, typename E> Either<E, T> runValidationRecursive(const std::vector<std::function<Either<E, T>(T)>> &Steps, size_t Index, T Current) */
template <typename T, typename E>
Either<E, T>
runValidationRecursive(const std::vector<std::function<Either<E, T>(T)>> &Steps,
                       size_t Index, T Current) {
  return Index == Steps.size()
             ? make_right(E{}, Current)
             : runValidationStep<T, E>(Steps, Index, Current);
}
} // namespace detail

/**
 * @fn template <typename T, typename E> Either<E, T> runValidation(const ValidationPipeline<T, E> &Pipeline, T Value)
 * @brief Runs validators in order and stops on the first error.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As validation flows, I need short-circuit execution so failing
 * input stops at the first invalid step.
 */
template <typename T, typename E>
Either<E, T> runValidation(const ValidationPipeline<T, E> &Pipeline, T Value) {
  return detail::runValidationRecursive<T, E>(Pipeline.Validators, 0,
                                              std::move(Value));
}

} // namespace func
