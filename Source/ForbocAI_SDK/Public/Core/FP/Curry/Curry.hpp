#pragma once

#include "Core/FP/Sequence/Sequence.hpp"

namespace func {
/**
 * @brief 5. CALLABLE: Curried (Function Currying) Converts an N-arity function into a chain of single-argument applications. Construction: use the curry<N>() factory function. operator() is the C++ mechanism for callable types (equivalent to lambda application in FP). Usage: auto add = [](int a, int b) { return a + b; }; auto curried = func::curry<2>(add); auto add5 = curried(5);    // partial application int result = add5(3);       // 8
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <size_t Arity, typename Func, typename CapturedArgs = std::tuple<>> struct Curried
 *
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <size_t Arity, typename Func, typename CapturedArgs = std::tuple<>>
struct Curried {
  Func func;
  CapturedArgs args;

/**
 * @brief Partial application: not enough args yet
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <typename... NewArgs> auto operator()(NewArgs &&...new_args) const -> typename std::enable_if< (std::tuple_size<CapturedArgs>::value + sizeof...(NewArgs) < Arity), Curried<Arity, Func, decltype(std::tuple_cat(args, std::make_tuple(std::forward<NewArgs>( new_args)...)))>>::type
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
  template <typename... NewArgs>
  auto operator()(NewArgs &&...new_args) const -> typename std::enable_if<
      (std::tuple_size<CapturedArgs>::value + sizeof...(NewArgs) < Arity),
      Curried<Arity, Func,
              decltype(std::tuple_cat(args,
                                      std::make_tuple(std::forward<NewArgs>(
                                          new_args)...)))>>::type {
    auto merged = std::tuple_cat(
        args, std::make_tuple(std::forward<NewArgs>(new_args)...));
    return Curried<Arity, Func, decltype(merged)>{func, merged};
  }

/**
 * @fn template <typename... NewArgs> auto operator()(NewArgs &&...new_args) const -> typename std::enable_if< (std::tuple_size<CapturedArgs>::value + sizeof...(NewArgs) >= Arity), decltype(func::apply(func, std::tuple_cat(args, std::make_tuple(std::forward<NewArgs>( new_args)...))))>::type
 * @brief Full application: enough args, invoke the function
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
  template <typename... NewArgs>
  auto operator()(NewArgs &&...new_args) const -> typename std::enable_if<
      (std::tuple_size<CapturedArgs>::value + sizeof...(NewArgs) >= Arity),
      decltype(func::apply(func,
                     std::tuple_cat(args, std::make_tuple(std::forward<NewArgs>(
                                              new_args)...))))>::type {
    return func::apply(
        func, std::tuple_cat(
                  args, std::make_tuple(std::forward<NewArgs>(new_args)...)));
  }
};

/**
 * @fn template <size_t Arity, typename Func> Curried<Arity, Func> curry(Func f)
 * @brief Converts a callable into a curried wrapper with the requested arity.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As functional composition code, I need currying so larger
 * runtime helpers can be partially applied in readable C++11.
 */
template <size_t Arity, typename Func> Curried<Arity, Func> curry(Func f) {
  return Curried<Arity, Func>{f, std::tuple<>{}};
}

template <typename A, typename B, typename R>
std::function<R(B)> partial_apply(std::function<R(A, B)> f, A a) {
  return [f, a](B b) { return f(a, b); };
}

template <typename A, typename B, typename C, typename R>
std::function<R(C)> partial_apply2(std::function<R(A, B, C)> f, A a, B b) {
  return [f, a, b](C c) { return f(a, b, c); };
}


} // namespace func
