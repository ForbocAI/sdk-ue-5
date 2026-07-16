#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
template <typename Arg, typename Result> struct Bounce {
  bool bDone;
  Arg next;
  Result result;
};

/**
 * @fn template <typename Arg, typename Result> Bounce<Arg, Result> call(Arg next)
 * @brief Returns a Bounce object representing the next step in a trampoline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a functional programmer, I need a way to return the next step in a recursive function to avoid stack overflows.
 */
template <typename Arg, typename Result>
Bounce<Arg, Result> call(Arg next) {
  return Bounce<Arg, Result>{false, std::move(next), Result{}};
}

/**
 * @fn template <typename Arg, typename Result> Bounce<Arg, Result> done(Result result)
 * @brief Returns a Bounce object representing the final result of a trampoline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a functional programmer, I need a way to return the final result of a recursive function to end the trampoline execution.
 */
template <typename Arg, typename Result>
Bounce<Arg, Result> done(Result result) {
  return Bounce<Arg, Result>{true, Arg{}, std::move(result)};
}

/**
 * @fn template <typename Arg, typename Result, typename Step> Result trampoline(Arg arg, Step step)
 * @brief Executes a trampoline to evaluate a recursive function without blowing the stack.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a functional programmer, I need a trampoline to safely execute deep recursive algorithms.
 */
template <typename Arg, typename Result, typename Step>
Result trampoline(Arg arg, Step step) {
  Bounce<Arg, Result> StepResult = step(std::move(arg));
  return StepResult.bDone
             ? StepResult.result
             : trampoline<Arg, Result, Step>(std::move(StepResult.next), step);
}

} // namespace func
