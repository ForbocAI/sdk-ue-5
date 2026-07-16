#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {

/**
 * @brief 1. HELPER: Index Sequence (C++14 backport) Generates a compile-time integer sequence for unpacking tuples. Equivalent to C++14's std::index_sequence / std::make_index_sequence. Note: gen_seq uses recursive template inheritance as the standard C++11 technique for this pattern.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * @signature template <size_t... Is> struct seq
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <size_t... Is> struct seq {};

template <size_t N, size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

template <size_t... Is> struct gen_seq<0, Is...> : seq<Is...> {};

/**
 * @fn template <typename F, typename Tuple, size_t... Is> auto apply_impl(F &&f, Tuple &&t, seq<Is...>) -> decltype(std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))...))
 * @brief Invokes a callable with tuple elements expanded by index sequence.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As C++11 functional helpers, I need tuple expansion so stored
 * argument lists can be replayed through generic callables cleanly.
 */
template <typename F, typename Tuple, size_t... Is>
auto apply_impl(F &&f, Tuple &&t, seq<Is...>)
    -> decltype(std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))...)) {
  return std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))...);
}

/**
 * @fn template <typename F, typename Tuple> auto apply(F &&f, Tuple &&t) -> decltype(apply_impl( std::forward<F>(f), std::forward<Tuple>(t), gen_seq<std::tuple_size< typename std::remove_reference<Tuple>::type>::value>()))
 * @brief Applies a callable to the contents of a tuple.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As higher-order helpers, I need tuple application so currying
 * and deferred calls can execute stored arguments consistently.
 */
template <typename F, typename Tuple>
auto apply(F &&f, Tuple &&t) -> decltype(apply_impl(
    std::forward<F>(f), std::forward<Tuple>(t),
    gen_seq<std::tuple_size<
        typename std::remove_reference<Tuple>::type>::value>())) {
  return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                    gen_seq<std::tuple_size<
                        typename std::remove_reference<Tuple>::type>::value>());
}

} // namespace func
