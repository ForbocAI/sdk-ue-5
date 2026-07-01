#pragma once
#ifndef UE_FP_HPP
#define UE_FP_HPP

/**
 * @brief UE FP Core Library — Strict UE C++11 functional programming primitives. No C++14, C++17, or later language features are used; Unreal container overloads are first-class because this SDK is UE-only. This header is the canonical source of truth for the functional substrate. If surrounding docs disagree, this file wins. DESIGN PRINCIPLES: - Prefer structs and plain data for domain state. - Prefer factory functions for construction of public values. - Keep domain behavior in free functions under the `func` namespace. - Use compatibility member wrappers only when preserving an existing callable C++11 surface is materially cheaper than duplicating abstractions (`MemoizedLast::operator()`, `AsyncResult` chaining). - Value semantics throughout. CONTENTS: 1. seq / gen_seq        — Index sequence (C++14 backport) 2. apply                — Tuple application (C++17 backport) 3. Maybe<T>             — Optional monad (data only) 4. Either<E, T>         — Result/Error monad (data only) 5. Curried / curry      — Automatic function currying 6. Lazy<T> / lazy       — Memoized deferred evaluation 7. MemoizedLast         — Last-input memoization for derived values 8. Pipeline<T> / pipe   — Value transformation chains (operator|) 9. Composed / compose   — Binary function composition 10. fmap                 — Functor map (Maybe, Either, vector, TArray) 11. mbind / ebind        — Monadic bind for Maybe / Either 12. or_else / match      — Extraction / pattern matching 13. ValidationPipeline   — Functional validation chain 14. ConfigBuilder        — Functional configuration builder 15. TestResult           — Functional testing result 16. AsyncResult          — Functional async result handling 17. HttpResult           — Functional HTTP result wrapper 18. AsyncChain           — AsyncResult chaining helpers 19. Dispatcher            — Dictionary-based typed dispatch 20. multi_match           — Multi-case value-based pattern matching 21. from_nullable         — Lift nullable values into Maybe 22. Unreal containers     — TArray and TMap folds, maps, traversal, lookup, update, equality REQUIREMENTS: Several helpers default-construct inactive payloads or error branches as a deliberate C++11 trade-off: `Maybe<T>`, `Either<E, T>`, `ValidationPipeline<T, E>`, `TestResult<T>`, and `HttpResult<T>`. All host types used with these primitives are expected to satisfy that requirement. See also: C++11-FP-GUIDE.md for patterns and usage.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 * Branching policy: do not mechanically replace `if` statements with ternary
 * chains. Domain alternatives should use `match` for Maybe/Either/result
 * values, `Dispatcher` or `multi_match` case tables for enum/string routing,
 * and map/filter/fold/find/traverse helpers for collection decisions. Explicit
 * guards are reserved for effect boundaries.
 * 
 * @signature #include <cstdint> #include <cstdlib> #include <functional> #include <memory> #include <stdexcept> #include <string> #include <tuple> #include <type_traits> #include <unordered_map> #include <utility> #include <vector> namespace func
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

#include "CoreMinimal.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

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
 * @brief Invokes a callable with tuple elements expanded by index sequence.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename F, typename Tuple, size_t... Is> auto apply_impl(F &&f, Tuple &&t, seq<Is...>) -> decltype(std::forward<F>(f)(std::get<Is>(std::forward<Tuple>(t))...))
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
 * @brief Applies a callable to the contents of a tuple.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename F, typename Tuple> auto apply(F &&f, Tuple &&t) -> decltype(apply_impl( std::forward<F>(f), std::forward<Tuple>(t), gen_seq<std::tuple_size< typename std::remove_reference<Tuple>::type>::value>()))
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
 * @brief Wraps a concrete value in a populated Maybe.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> Maybe<T> just(T v)
 * 
 * User Story: As optional flows, I need a simple way to lift a value into
 * Maybe so absence and presence stay explicit in pipelines.
 */
template <typename T> Maybe<T> just(T v) {
  return Maybe<T>{true, std::move(v)};
}

/**
 * @brief Builds an empty Maybe with a default-constructed payload.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> Maybe<T> nothing()
 * 
 * User Story: As optional flows, I need a canonical empty Maybe so code can
 * represent missing values without custom sentinels.
 */
template <typename T> Maybe<T> nothing() { return Maybe<T>{false, T{}}; }

/**
 * @brief Returns true when a Maybe contains a value.
 *
 * @signature template <typename T> bool is_just(const Maybe<T> &m)
 *
 * User Story: As a cross-SDK caller, I need an explicit presence predicate so
 * optional flows read the same way in C++, Rust, TypeScript, and GDScript.
 */
template <typename T> bool is_just(const Maybe<T> &m) { return m.hasValue; }

/**
 * @brief Returns true when a Maybe is empty.
 *
 * @signature template <typename T> bool is_nothing(const Maybe<T> &m)
 *
 * User Story: As a boundary author, I need an explicit absence predicate so
 * fallback branches are visible instead of hidden behind raw struct fields.
 */
template <typename T> bool is_nothing(const Maybe<T> &m) {
  return !m.hasValue;
}

/**
 * @brief Keeps a Maybe value only when it satisfies a predicate.
 *
 * @signature template <typename T, typename Predicate> Maybe<T> maybe_filter(const Maybe<T> &m, Predicate predicate)
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
 * @brief Converts a host Maybe-shaped value into the functional Maybe type.
 *
 * @signature template <typename T> Maybe<T> maybe_from_option(Maybe<T> opt)
 *
 * User Story: As an interop author, I need a named bridge that mirrors Rust
 * Option conversion even though C++11 uses the same Maybe data shape here.
 */
template <typename T> Maybe<T> maybe_from_option(Maybe<T> opt) {
  return opt;
}

/**
 * @brief Converts a functional Maybe back to the host option shape.
 *
 * @signature template <typename T> Maybe<T> maybe_to_option(Maybe<T> maybe)
 *
 * User Story: As an SDK maintainer, I need a symmetrical lowering helper so
 * examples and generated docs can use the same names across languages.
 */
template <typename T> Maybe<T> maybe_to_option(Maybe<T> maybe) {
  return maybe;
}

/**
 * @brief 4. DATA: Either (Result/Error Monad) A sum type for computations that can fail. Convention: Left = error, Right = success. Pure data struct — no methods. Construction: use factory functions make_left() / make_right() Operations:   use free functions fmap() / ebind() Requires: E and T are default-constructible.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T> struct Either
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename E, typename T> struct Either {
  bool isLeft;
  E left;
  T right;
};

/**
 * @brief Constructs the error branch of an Either value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T> Either<E, T> make_left(E e)
 * 
 * User Story: As result-returning code, I need a clear error constructor so
 * failure paths remain explicit in functional chains.
 */
template <typename E, typename T> Either<E, T> make_left(E e) {
  return Either<E, T>{true, std::move(e), T{}};
}

/**
 * @brief Constructs the error branch while preserving an explicit fallback payload.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T> Either<E, T> make_left(E e, T dummy)
 * 
 * User Story: As result-returning code, I need an error constructor that also
 * satisfies payload shape requirements in C++11.
 */
template <typename E, typename T> Either<E, T> make_left(E e, T dummy) {
  return Either<E, T>{true, std::move(e), std::move(dummy)};
}

/**
 * @brief Constructs the success branch of an Either value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T> Either<E, T> make_right(T v)
 * 
 * User Story: As result-returning code, I need a clear success constructor so
 * successful values move through pipelines predictably.
 */
template <typename E, typename T> Either<E, T> make_right(T v) {
  return Either<E, T>{false, E{}, std::move(v)};
}

/**
 * @brief Constructs the success branch while preserving an explicit fallback error value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T> Either<E, T> make_right(E dummy, T v)
 * 
 * User Story: As result-returning code, I need a success constructor that also
 * preserves error shape requirements in C++11.
 */
template <typename E, typename T> Either<E, T> make_right(E dummy, T v) {
  return Either<E, T>{false, std::move(dummy), std::move(v)};
}

/**
 * @brief Alias for constructing the error branch of an Either.
 *
 * @signature template <typename E, typename T> Either<E, T> left(E e)
 *
 * User Story: As a functional-programming caller, I need the common Left name
 * so failure construction matches Rust, TypeScript, and GDScript examples.
 */
template <typename E, typename T> Either<E, T> left(E e) {
  return make_left<E, T>(std::move(e));
}

/**
 * @brief Alias for constructing the success branch of an Either.
 *
 * @signature template <typename E, typename T> Either<E, T> right(T v)
 *
 * User Story: As a functional-programming caller, I need the common Right name
 * so successful result construction is portable across SDK cores.
 */
template <typename E, typename T> Either<E, T> right(T v) {
  return make_right<E, T>(std::move(v));
}

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
 * @brief Full application: enough args, invoke the function
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename... NewArgs> auto operator()(NewArgs &&...new_args) const -> typename std::enable_if< (std::tuple_size<CapturedArgs>::value + sizeof...(NewArgs) >= Arity), decltype(func::apply(func, std::tuple_cat(args, std::make_tuple(std::forward<NewArgs>( new_args)...))))>::type
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
 * @brief Converts a callable into a curried wrapper with the requested arity.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <size_t Arity, typename Func> Curried<Arity, Func> curry(Func f)
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


/**
 * @brief 6. DATA: Lazy (Deferred Evaluation) Memoized deferred computation. The thunk is evaluated at most once on first access via eval(). Construction: use the lazy() factory function. Access:       use the eval() free function. Note: Not thread-safe. Intended for single-thread use (e.g. game thread).
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> struct Lazy
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct Lazy {
  std::function<T()> thunk;
  mutable std::shared_ptr<T> cached;
};

/**
 * @brief Wraps a thunk so it is evaluated once on first access.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename F> auto lazy(F &&f) -> Lazy<decltype(f())>
 * 
 * User Story: As deferred setup code, I need lazy values so expensive work only
 * runs when a caller actually needs the result.
 */
template <typename F> auto lazy(F &&f) -> Lazy<decltype(f())> {
  return Lazy<decltype(f())>{std::forward<F>(f), nullptr};
}

namespace detail {
template <typename T> const T &cacheLazyValue(const Lazy<T> &lz) {
  lz.cached = std::make_shared<T>(lz.thunk());
  return *lz.cached;
}
} // namespace detail

/**
 * @brief Forces a lazy value and memoizes the computed result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> const T &eval(const Lazy<T> &lz)
 * 
 * User Story: As deferred setup code, I need an explicit force helper so lazy
 * values can be materialized through one consistent API.
 */
template <typename T> const T &eval(const Lazy<T> &lz) {
  return lz.cached ? *lz.cached : detail::cacheLazyValue(lz);
}

/**
 * @brief 7. CALLABLE: MemoizedLast (Last-Input Memoization) Memoizes the most recent invocation of a pure function. This is the canonical primitive for selector-style derived-data memoization. Construction: use the memoizeLast<Signature>() factory function, optionally with a custom comparator. Access: call the wrapper like a normal function. Note: the default comparator uses tuple equality, so callers with non-comparable or overly-large inputs should supply a custom comparator over a smaller key.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Signature> struct MemoizedLast
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename Signature> struct MemoizedLast;

namespace detail {
template <typename Signature> struct MemoizedLastFactory;

template <typename Result, typename... Args>
const Result &callMemoizedLast(const MemoizedLast<Result(Args...)> &memoized,
                               Args... args);
} // namespace detail

template <typename Result, typename... Args>
struct MemoizedLast<Result(Args...)> {
  typedef std::tuple<typename std::decay<Args>::type...> ArgsTuple;
  typedef std::function<bool(const ArgsTuple &, const ArgsTuple &)> Comparator;
  std::function<Result(Args...)> func;
  Comparator equals;
  mutable bool hasCached = false;
  mutable ArgsTuple lastArgs;
  mutable std::shared_ptr<Result> lastResult;

  const Result &operator()(Args... args) const {
    return detail::callMemoizedLast(*this, std::forward<Args>(args)...);
  }
};

namespace detail {
template <typename Result, typename... Args>
struct MemoizedLastFactory<Result(Args...)> {
  typedef MemoizedLast<Result(Args...)> MemoizedType;

  static typename MemoizedType::Comparator defaultComparator() {
    return typename MemoizedType::Comparator(
        [](const typename MemoizedType::ArgsTuple &lhs,
           const typename MemoizedType::ArgsTuple &rhs) { return lhs == rhs; });
  }

  static MemoizedType
  create(std::function<Result(Args...)> function,
         typename MemoizedType::Comparator comparator = defaultComparator()) {
    MemoizedType memoized;
    memoized.func = std::move(function);
    memoized.equals = std::move(comparator);
    return memoized;
  }
};

template <typename Result>
const Result &storeMemoizedResult(std::shared_ptr<Result> &target,
                                  Result computed) {
  return target ? (*target = std::move(computed), *target)
                : *(target = std::make_shared<Result>(std::move(computed)));
}

template <typename Result, typename... Args>
const Result &
recomputeMemoizedLast(const MemoizedLast<Result(Args...)> &memoized,
                      typename MemoizedLast<Result(Args...)>::ArgsTuple current) {
  Result computed = func::apply(memoized.func, current);
  memoized.lastArgs = current;
  memoized.hasCached = true;
  return storeMemoizedResult(memoized.lastResult, std::move(computed));
}

template <typename Result, typename... Args>
const Result &callMemoizedLast(const MemoizedLast<Result(Args...)> &memoized,
                               Args... args) {
  typename MemoizedLast<Result(Args...)>::ArgsTuple current(
      std::forward<Args>(args)...);
  bool useCached = memoized.hasCached && memoized.lastResult &&
                   memoized.equals(memoized.lastArgs, current);
  return useCached ? *memoized.lastResult
                   : recomputeMemoizedLast(memoized, std::move(current));
}
} // namespace detail

/**
 * @brief Memoizes the last invocation of a std::function with default comparison.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Signature> MemoizedLast<Signature> memoizeLast(std::function<Signature> function)
 * 
 * User Story: As derived-data helpers, I need last-call memoization so cached
 * computations can be reused when inputs repeat.
 */
template <typename Signature>
MemoizedLast<Signature> memoizeLast(std::function<Signature> function) {
  return detail::MemoizedLastFactory<Signature>::create(std::move(function));
}

/**
 * @brief Memoizes the last invocation of a generic callable with default comparison.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Signature, typename F> MemoizedLast<Signature> memoizeLast(F f)
 * 
 * User Story: As derived-data helpers, I need memoization for generic
 * callables so caching is not limited to std::function inputs.
 */
template <typename Signature, typename F>
MemoizedLast<Signature> memoizeLast(F f) {
  return detail::MemoizedLastFactory<Signature>::create(
      std::function<Signature>(f));
}

/**
 * @brief Memoizes the last invocation using a custom argument comparator.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Signature> MemoizedLast<Signature> memoizeLast(std::function<Signature> function, typename MemoizedLast<Signature>::Comparator comparator)
 * 
 * User Story: As derived-data helpers, I need custom comparison so caching can
 * respect caller-defined notions of argument equality.
 */
template <typename Signature>
MemoizedLast<Signature>
memoizeLast(std::function<Signature> function,
            typename MemoizedLast<Signature>::Comparator comparator) {
  return detail::MemoizedLastFactory<Signature>::create(std::move(function),
                                                        std::move(comparator));
}

/**
 * @brief Memoizes a generic callable using a custom argument comparator.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Signature, typename F> MemoizedLast<Signature> memoizeLast(F f, typename MemoizedLast<Signature>::Comparator comparator)
 * 
 * User Story: As derived-data helpers, I need generic custom-comparator
 * memoization so reusable callables can control cache invalidation.
 */
template <typename Signature, typename F>
MemoizedLast<Signature>
memoizeLast(F f, typename MemoizedLast<Signature>::Comparator comparator) {
  return detail::MemoizedLastFactory<Signature>::create(
      std::function<Signature>(f), std::move(comparator));
}

/**
 * @brief 8. DATA: Pipeline (Value Transformation) Fluent chain for threading a value through a series of pure transformations using operator|. Construction: use the pipe() factory function. Chaining:     use operator| with transform functions. Extraction:   access the .val member directly. Usage: auto add1 = [](int x) { return x + 1; }; auto mul2 = [](int x) { return x * 2; }; auto result = func::pipe(5) | add1 | mul2; int final = result.val;  // 12
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> struct Pipeline
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename T> struct Pipeline {
  T val;
};

/**
 * @brief Starts a pipeline with an initial value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> Pipeline<T> pipe(T v)
 * 
 * User Story: As functional composition code, I need a pipeline entry point so
 * value-threading reads clearly in C++11 call sites.
 */
template <typename T> Pipeline<T> pipe(T v) {
  return Pipeline<T>{std::move(v)};
}

template <typename F> struct Tapped {
  F f;

  template <typename T> T operator()(T value) {
    f(value);
    return value;
  }
};

template <typename F> Tapped<F> tap(F f) { return Tapped<F>{std::move(f)}; }

template <typename F> struct TapMut {
  F f;

  template <typename T> T &operator()(T &value) {
    f(value);
    return value;
  }
};

template <typename F> TapMut<F> tap_mut(F f) {
  return TapMut<F>{std::move(f)};
}

/**
 * @brief operator| for chaining lvalue-backed pipelines.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename F> auto operator|(const Pipeline<T> &p, F f) -> Pipeline<decltype(f(p.val))>
 * 
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename T, typename F>
auto operator|(const Pipeline<T> &p, F f) -> Pipeline<decltype(f(p.val))> {
  return Pipeline<decltype(f(p.val))>{f(p.val)};
}

/**
 * @brief operator| for chaining move-only or ownership-transferring values.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename F> auto operator|(Pipeline<T> &&p, F f) -> Pipeline<decltype(f(std::move(p.val)))>
 * 
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename T, typename F>
auto operator|(Pipeline<T> &&p, F f)
    -> Pipeline<decltype(f(std::move(p.val)))> {
  return Pipeline<decltype(f(std::move(p.val)))>{f(std::move(p.val))};
}

/**
 * @brief 9. CALLABLE: Composed (Function Composition) Combines two functions: compose(f, g)(x) == f(g(x)) Construction: use the compose() factory function. operator() is the C++ mechanism for callable types. Usage: auto double_it = [](int x) { return x * 2; }; auto add_one   = [](int x) { return x + 1; }; auto both      = func::compose(add_one, double_it); int result = both(5);  // add_one(double_it(5)) = 11
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename F, typename G> struct Composed
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

template <typename F, typename G> struct Composed {
  F f;
  G g;

  template <typename... Args>
  auto operator()(Args &&...args) const
      -> decltype(f(g(std::forward<Args>(args)...))) {
    return f(g(std::forward<Args>(args)...));
  }
};

/**
 * @brief Composes two functions so the result of `g` feeds into `f`.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename F, typename G> Composed<F, G> compose(F f, G g)
 * 
 * User Story: As functional composition code, I need reusable composition so
 * runtime transforms can be assembled declaratively.
 */
template <typename F, typename G> Composed<F, G> compose(F f, G g) {
  return Composed<F, G>{f, g};
}


/**
 * @brief Unary-composition cookbook: neutral point-free helpers built on the
 * existing FP core so feature code can compose small reusable functions
 * instead of inventing domain-shaped substitutes for RTK or ECS.
 *
 * @signature template helpers: identity, constant, flip, complement, both,
 * either_pred, all_pass, any_pass, converge2, juxt2, pipe3, pipe4,
 * catalog, fold_catalog, zip_catalog_fold, tail_tuple, fold_vector,
 * filter_vector, find_vector, contains_value, unique_by, traverse_maybe,
 * sequence_maybe, fold_either, lift2, lift3, eq_by, has_key
 *
 * User Story: As a cross-SDK maintainer, I need the same unary-composition
 * recipes available in C++ as Rust, GDScript, and TypeScript so reducers,
 * selectors, and ECS systems can share examples and semantics.
 *
 * Cookbook rules:
 * - Functional core supplies pure composition, lazy values, Maybe/Either,
 *   predicate builders, collection transforms, and strict dispatch helpers.
 * - RTK owns the single store, event-style actions, reducers, selectors,
 *   thunks, adapters, and unidirectional data flow.
 * - ECS owns entity/component/system/world value transitions. ECS may use
 *   functional helpers, but functional helpers must not become a parallel
 *   action/store/reducer model.
 * - Views dispatch actions and read selectors. They do not own reducer logic,
 *   ECS query decisions, component matching, or derived domain state.
 * - Prefer reusable factories that return unary functions:
 *     auto HasTag = requireTag(World, Tag);
 *     auto InDomain = requireDomain(World, Domain);
 *     auto Matches = func::all_pass<EntityKey>({HasTag, InDomain});
 * - Do not stop at a repeated family of typed factories. If call sites still
 *   repeat `BindBool<Domain>(Name, Reader)`, `BindText<Domain>(Name, Reader)`,
 *   `ComponentField(Name, Member)`, `field_names(...)` plus
 *   `field_readers(...)`, or equivalent noun/type plumbing, push the
 *   abstraction down another level. The lower layer should compose functions
 *   into functions:
 *     domain declaration -> registry lookup -> value transform -> fold
 *   so higher domains provide only declaration data and the minimal
 *   domain-specific facts. The target is a composed unary transform built from
 *   smaller composed unary transforms, not a nicer-looking wrapper around the
 *   same repeated call shape.
 *
 *   Definitive generic style: name the composition boundary once, then pass a
 *   collection of irreducible domain atoms to one reusable composer. The source
 *   type, reducer slice, validator target, or dispatch target should come from
 *   the surrounding generic composer/context. Do not invent noun wrappers such
 *   as `FTownspersonSeeds(...)` just to forward a field list.
 *
 *   📣 MEGAPHONE RULE: a generic helper repeated once per field/member is still
 *   the wrong shape. Do not write `SettingField(...)`, `Required(...)`,
 *   `ComponentField(...)`, `BindFloat(...)`, or equivalent per-field wrapper
 *   calls at feature call sites. Settings, parser, projection, validation, and
 *   formatter code should pass grouped declaration atoms such as
 *   `ReadSettingsFields<FPlayerPresentationSettings>(
 *       Object, {"CapsuleRadius", "CapsuleHalfHeight", ...})`.
 *   The reusable composer owns `TEXT(...)`, snake_case/path conversion,
 *   typed reader selection, validation, traversal, and fold expansion.
 *   Digit-suffixed helper or macro families such as `READ_FIELD_1`,
 *   `READ_FIELD_2`, `FIELDS_32`, or `GET_FIELD_READER_10` are not
 *   compositional; they are arity plumbing hidden in names. Use one named
 *   composition boundary plus grouped declaration data instead.
 *
 *   This rule applies at every layer, including registries. A registry,
 *   adapter, validator, formatter, selector, reducer helper, or execution
 *   runner is not permission to hand-write
 *   `compose(ReadMember(A), ReadMember(B))` chains. Source/target lookup must
 *   also be declaration-shaped: use atoms, paths, conversions, predicates, and
 *   cases as data, then let one generic composer expand those declarations
 *   into readers, projectors, validators, dispatchers, reducers, or folds.
 *
 *   Definitive grouped declaration shape: the outer function owns the inner
 *   expansion. A call site supplies grouped atoms and paths, not nested helper
 *   calls:
 *     RegisteredGroups({
 *         {"Runtime/Bots",
 *          {"HasActiveGoal",
 *           "ActiveGoal",
 *           "GoalQueue",
 *           {"KnownLandmarkIds", {"Knowledge", "KnownLandmarkIds"}},
 *           {"KnownBotIds", {"Knowledge", "KnownBotIds"}}}},
 *         {"Runtime/Spatial", {"LocalLocation", "WorldLocation"}}});
 *
 *   A narrower boundary can accept only field/path atoms when the surrounding
 *   composer already knows the group:
 *     RegisteredFields({
 *         {"KnownLandmarkIds", {"Knowledge", "KnownLandmarkIds"}},
 *         {"KnownBotIds", {"Knowledge", "KnownBotIds"}}});
 *
 *   Converted values follow the same rule: declare the source path and the
 *   conversion function as data, then let the generic composer build the unary
 *   transform. This applies to component projection, validation, formatting,
 *   selector derivation, reducer payload shaping, and execution pipelines:
 *     RegisteredFields({
 *         "BehaviorState", {"BehaviorState"}, Convert(BotBehaviorText),
 *         "KnownBotIds", {"Knowledge", "KnownBotIds"}});
 *
 *   Domain grouping is still generic declaration data. The boundary function
 *   owns the inner helper expansion; do not nest `RegisteredFields`,
 *   `RegisteredRules`, `Required`, `Sequence`, or similar helper calls inside
 *   the declaration unless the argument is genuinely custom/one-off behavior:
 *     RegisteredDomain(
 *         "Runtime/Bots",
 *         {"Persona",
 *          "InteractionPrompt",
 *          "DefaultPlayerLine",
 *          "PinnedResponse",
 *          "InteractionIntent"});
 *
 *     RegisteredDomain(
 *         "Validation/Bots",
 *         {"Id", "Persona", "Goals"},
 *         {"Type", "Priority", "TargetEntityId"});
 *
 *     RegisteredCases({"Entity", "Component", "Relationship", "System"});
 *     RegisteredProjectionPasses({"Terrain", "Spawn", "Townsperson"});
 *
 *   Generic declaration helpers own platform string conversion. Do not repeat
 *   `TEXT(...)` around every atom and do not hide that repetition behind a
 *   noun wrapper. Do not replace field/path declarations with per-field
 *   `ReadMember`, `ComponentField`, `BindText`, or hand-composed reader
 *   chains.
 *
 *   The same rule applies to every repeated composition idiom, not only member
 *   reading: path assembly, entity selection, source selection, value
 *   conversion, binding construction, validation, formatting, lookup,
 *   traversal, and execution folds. If a call site repeats
 *   `ComposeX(Names, Readers)`, `ProjectPayloadY(Entity, Source, Fields)`, or
 *   matching lambda chains across domains, that is still an abstraction leak.
 *   Move the repeated shape into a higher composer and leave the current layer
 *   with grouped declarations. If the current layer is a registry, it still
 *   declares fields/paths/conversions as data.
 *
 *   Rule of thumb: if a feature file repeats a function call per field, per
 *   type, per domain prefix, per selector/projector pair, per validation rule,
 *   per formatting case, or per execution pass, it is not abstracted far
 *   enough. Move that shape into one composer and feed it grouped declaration
 *   data. Repetition is acceptable only for irreducible domain atoms inside
 *   those declarations.
 * - Use request/payload structs at public domain boundaries and for genuine
 *   multi-input domain operations. Do not create a new request struct when a
 *   reusable unary function, predicate, fold, lift, or traversal is the actual
 *   abstraction.
 * - When only nouns change, put selectors, projectors, validators, or
 *   transforms in registries/catalogs and fold those with generic declaration
 *   functions. Do not create `FNounThing(...)` wrapper families that only
 *   forward names into the same composer. When two noun lists advance
 *   together, use zip_catalog_fold so recursion is one primitive instead of a
 *   family of domain-shaped wrappers.
 * - Neutral primitives live below feature domains. Feature domains import
 *   downward into these primitives instead of borrowing helpers from siblings.
 * - Hidden fallback behavior belongs at explicit integration boundaries only.
 *   Reducers, selectors, and ECS systems should prefer Maybe/Either-returning
 *   helpers so missing data is visible in the type.
 */

template <typename T> T identity(T value) { return value; }

template <typename T> struct Constant {
  T Value;
  template <typename A> T operator()(const A &) const { return Value; }
};

template <typename T> Constant<T> constant(T value) {
  return Constant<T>{value};
}

template <typename F> struct Flipped {
  F Fn;
  template <typename B, typename A>
  auto operator()(B b, A a) const -> decltype(Fn(a, b)) {
    return Fn(a, b);
  }
};

template <typename F> Flipped<F> flip(F fn) { return Flipped<F>{fn}; }

template <typename Predicate> struct Complement {
  Predicate Fn;
  template <typename T> bool operator()(const T &value) const {
    return !Fn(value);
  }
};

template <typename Predicate> Complement<Predicate> complement(Predicate fn) {
  return Complement<Predicate>{fn};
}

template <typename LeftPredicate, typename RightPredicate> struct Both {
  LeftPredicate Left;
  RightPredicate Right;
  template <typename T> bool operator()(const T &value) const {
    return Left(value) && Right(value);
  }
};

template <typename LeftPredicate, typename RightPredicate>
Both<LeftPredicate, RightPredicate> both(LeftPredicate left,
                                         RightPredicate right) {
  return Both<LeftPredicate, RightPredicate>{left, right};
}

template <typename LeftPredicate, typename RightPredicate> struct EitherPred {
  LeftPredicate Left;
  RightPredicate Right;
  template <typename T> bool operator()(const T &value) const {
    return Left(value) || Right(value);
  }
};

template <typename LeftPredicate, typename RightPredicate>
EitherPred<LeftPredicate, RightPredicate> either_pred(LeftPredicate left,
                                                      RightPredicate right) {
  return EitherPred<LeftPredicate, RightPredicate>{left, right};
}

template <typename T>
std::function<bool(const T &)>
all_pass(const std::vector<std::function<bool(const T &)>> &predicates) {
  return [predicates](const T &value) {
    std::function<bool(size_t)> eval = [&](size_t index) {
      return index >= predicates.size()
                 ? true
                 : predicates[index](value) && eval(index + 1);
    };
    return eval(0);
  };
}

template <typename T>
std::function<bool(const T &)>
any_pass(const std::vector<std::function<bool(const T &)>> &predicates) {
  return [predicates](const T &value) {
    std::function<bool(size_t)> eval = [&](size_t index) {
      return index >= predicates.size()
                 ? false
                 : predicates[index](value) || eval(index + 1);
    };
    return eval(0);
  };
}

template <typename Combine, typename First, typename Second> struct Converge2 {
  Combine CombineFn;
  First FirstFn;
  Second SecondFn;
  template <typename A>
  auto operator()(const A &value) const
      -> decltype(CombineFn(FirstFn(value), SecondFn(value))) {
    return CombineFn(FirstFn(value), SecondFn(value));
  }
};

template <typename Combine, typename First, typename Second>
Converge2<Combine, First, Second> converge2(Combine combine, First first,
                                            Second second) {
  return Converge2<Combine, First, Second>{combine, first, second};
}

template <typename First, typename Second> struct Juxt2 {
  First FirstFn;
  Second SecondFn;
  template <typename A>
  auto operator()(const A &value) const
      -> std::pair<decltype(FirstFn(value)), decltype(SecondFn(value))> {
    return std::make_pair(FirstFn(value), SecondFn(value));
  }
};

template <typename First, typename Second>
Juxt2<First, Second> juxt2(First first, Second second) {
  return Juxt2<First, Second>{first, second};
}

/**
 * @brief Returns true when a projected value equals the expected value.
 *
 * @signature template <typename Projection, typename Expected> EqBy<Projection, Expected> eq_by(Projection projection, Expected expected)
 *
 * User Story: As reducer and ECS predicate code, I need reusable projected
 * equality so query predicates compose without one-off request structs.
 */
template <typename Projection, typename Expected> struct EqBy {
  Projection Project;
  Expected ExpectedValue;

  template <typename T> bool operator()(const T &value) const {
    return Project(value) == ExpectedValue;
  }
};

template <typename Projection, typename Expected>
EqBy<Projection, Expected> eq_by(Projection projection, Expected expected) {
  return EqBy<Projection, Expected>{projection, expected};
}

/**
 * @brief Builds a unary predicate that checks whether a map contains a key.
 *
 * @signature template <typename Key, typename Value> std::function<bool(const Key &)> has_key(const std::unordered_map<Key, Value> &values)
 *
 * User Story: As validation and dispatcher code, I need map membership as a
 * reusable unary predicate so rule composition stays point-free.
 */
template <typename Key, typename Value>
std::function<bool(const Key &)>
has_key(const std::unordered_map<Key, Value> &values) {
  return [&values](const Key &key) { return values.find(key) != values.end(); };
}

template <typename F, typename G, typename H>
auto pipe3(F f, G g, H h) -> decltype(compose(h, compose(g, f))) {
  return compose(h, compose(g, f));
}

template <typename F, typename G, typename H, typename I>
auto pipe4(F f, G g, H h, I i) -> decltype(compose(i, pipe3(f, g, h))) {
  return compose(i, pipe3(f, g, h));
}

/**
 * @brief Stores a typed compile-time catalog of values or functions.
 *
 * @signature template <typename... Values> struct Catalog
 *
 * User Story: As feature code, I need noun-changing entries to become data
 * in one reusable list shape instead of spawning request/factory families.
 */
template <typename... Values> struct Catalog {
  std::tuple<Values...> values;
};

/**
 * @brief Builds a typed catalog from values or functions.
 *
 * @signature template <typename... Values> Catalog<decay<Values>...> catalog(Values &&...values)
 *
 * User Story: As composition code, I need a small variadic builder so repeated
 * selectors, projectors, validators, and transforms can be folded uniformly.
 */
template <typename... Values>
Catalog<typename std::decay<Values>::type...> catalog(Values &&...values) {
  return Catalog<typename std::decay<Values>::type...>{
      std::make_tuple(std::forward<Values>(values)...)};
}

namespace detail {
template <typename Tuple, size_t... Indices>
auto tailTupleImpl(const Tuple &values, seq<Indices...>)
    -> decltype(std::make_tuple(std::get<Indices + 1>(values)...)) {
  return std::make_tuple(std::get<Indices + 1>(values)...);
}
} // namespace detail

/**
 * @brief Returns a tuple containing every value after the head.
 *
 * @signature template <typename Head, typename... Tail> std::tuple<Tail...> tail_tuple(const std::tuple<Head, Tail...> &values)
 *
 * User Story: As catalog recursion, I need one neutral tuple-tail primitive so
 * higher folds do not reimplement tuple slicing per domain.
 */
template <typename Head, typename... Tail>
std::tuple<Tail...> tail_tuple(const std::tuple<Head, Tail...> &values) {
  return detail::tailTupleImpl(values, gen_seq<sizeof...(Tail)>());
}

namespace detail {
template <typename Acc, typename Step>
Acc foldTupleRecursive(const std::tuple<> &, Acc acc, Step) {
  return acc;
}

template <typename Head, typename... Tail, typename Acc, typename Step>
Acc foldTupleRecursive(const std::tuple<Head, Tail...> &values, Acc acc,
                       Step step) {
  return foldTupleRecursive(tail_tuple(values),
                            step(acc, std::get<0>(values)), step);
}

template <typename Acc, typename Step>
Acc zipTupleFoldRecursive(const std::tuple<> &, const std::tuple<> &, Acc acc,
                          Step) {
  return acc;
}

template <typename LeftHead, typename... LeftTail, typename RightHead,
          typename... RightTail, typename Acc, typename Step>
Acc zipTupleFoldRecursive(const std::tuple<LeftHead, LeftTail...> &left,
                          const std::tuple<RightHead, RightTail...> &right,
                          Acc acc, Step step) {
  return zipTupleFoldRecursive(
      tail_tuple(left), tail_tuple(right),
      step(acc, std::get<0>(left), std::get<0>(right)), step);
}
} // namespace detail

/**
 * @brief Folds a catalog through one accumulator step.
 *
 * @signature template <typename... Values, typename Acc, typename Step> Acc fold_catalog(const Catalog<Values...> &values, Acc seed, Step step)
 *
 * User Story: As ECS and RTK adapter code, I need function catalogs to execute
 * through one reusable recursion instead of repeated named wrapper families.
 */
template <typename... Values, typename Acc, typename Step>
Acc fold_catalog(const Catalog<Values...> &values, Acc seed, Step step) {
  return detail::foldTupleRecursive(values.values, seed, step);
}

/**
 * @brief Folds two catalogs in lockstep through one accumulator step.
 *
 * @signature template <typename... LeftValues, typename... RightValues, typename Acc, typename Step> Acc zip_catalog_fold(const Catalog<LeftValues...> &left, const Catalog<RightValues...> &right, Acc seed, Step step)
 *
 * User Story: As projection code, paired selector/projector lists should be
 * registered as data and executed by one neutral fold.
 */
template <typename... LeftValues, typename... RightValues, typename Acc,
          typename Step>
Acc zip_catalog_fold(const Catalog<LeftValues...> &left,
                     const Catalog<RightValues...> &right, Acc seed,
                     Step step) {
  static_assert(sizeof...(LeftValues) == sizeof...(RightValues),
                "zip_catalog_fold requires equally sized catalogs");
  return detail::zipTupleFoldRecursive(left.values, right.values, seed, step);
}

inline std::function<int(int)> example_clamp_channel() {
  return pipe3([](int value) { return std::max(0, value); },
               [](int value) { return std::min(255, value); }, identity<int>);
}

inline std::function<float(const std::pair<float, float> &)>
example_fill_ratio() {
  return converge2(
      [](float current, float maxValue) {
        return current / std::max(maxValue, 1.192092896e-07F);
      },
      [](const std::pair<float, float> &reading) { return reading.first; },
      [](const std::pair<float, float> &reading) { return reading.second; });
}

inline std::function<bool(const std::pair<bool, bool> &)>
example_alive_and_visible() {
  return both([](const std::pair<bool, bool> &flags) { return flags.first; },
              [](const std::pair<bool, bool> &flags) { return flags.second; });
}

namespace detail {
template <typename T, typename Acc, typename Step>
Acc foldVectorRecursive(const std::vector<T> &values, size_t index, Acc acc,
                        Step step) {
  return index == values.size()
             ? acc
             : foldVectorRecursive<T, Acc, Step>(
                   values, index + 1, step(acc, values[index]), step);
}

template <typename T, typename Predicate>
std::vector<T> filterVectorRecursive(const std::vector<T> &values,
                                     size_t index, Predicate predicate,
                                     std::vector<T> result) {
  return index == values.size()
             ? result
             : (predicate(values[index]) ? result.push_back(values[index])
                                         : void(),
                filterVectorRecursive<T, Predicate>(values, index + 1,
                                                    predicate,
                                                    std::move(result)));
}

template <typename T, typename Predicate>
Maybe<T> findVectorRecursive(const std::vector<T> &values, size_t index,
                             Predicate predicate) {
  return index == values.size()
             ? nothing<T>()
             : (predicate(values[index])
                    ? just(values[index])
                    : findVectorRecursive<T, Predicate>(values, index + 1,
                                                       predicate));
}

template <typename Key>
bool vectorContainsKey(const std::vector<Key> &values, const Key &key,
                       size_t index) {
  return index == values.size()
             ? false
             : (values[index] == key ||
                vectorContainsKey<Key>(values, key, index + 1));
}

template <typename T, typename Projection, typename Key>
std::vector<T> uniqueByRecursive(const std::vector<T> &values, size_t index,
                                 Projection projection,
                                 std::vector<Key> seen,
                                 std::vector<T> result) {
  return index == values.size()
             ? result
             : (vectorContainsKey<Key>(seen, projection(values[index]), 0)
                    ? uniqueByRecursive<T, Projection, Key>(
                          values, index + 1, projection, std::move(seen),
                          std::move(result))
                    : (seen.push_back(projection(values[index])),
                       result.push_back(values[index]),
                       uniqueByRecursive<T, Projection, Key>(
                           values, index + 1, projection, std::move(seen),
                           std::move(result))));
}

template <typename T, typename Func, typename U>
Maybe<std::vector<U>> traverseMaybeRecursive(const std::vector<T> &values,
                                             size_t index, Func fn,
                                             std::vector<U> result) {
  return index == values.size()
             ? just(result)
             : [&]() {
                 Maybe<U> current = fn(values[index]);
                 return current.hasValue
                            ? (result.push_back(current.value),
                               traverseMaybeRecursive<T, Func, U>(
                                   values, index + 1, fn, std::move(result)))
                            : nothing<std::vector<U>>();
               }();
}

template <typename E, typename T, typename Acc, typename Step>
Either<E, Acc> foldEitherRecursive(const std::vector<T> &values, size_t index,
                                   Acc acc, Step step) {
  return index == values.size()
             ? make_right<E, Acc>(acc)
             : ebind(step(acc, values[index]),
                     [&](const Acc &next) -> Either<E, Acc> {
                       return foldEitherRecursive<E, T, Acc, Step>(
                           values, index + 1, next, step);
                     });
}
} // namespace detail

/**
 * @brief Folds a vector through a pure accumulator step.
 *
 * @signature template <typename T, typename Acc, typename Step> Acc fold_vector(const std::vector<T> &values, Acc seed, Step step)
 *
 * User Story: As feature and ECS code, I need a reusable fold so repeated
 * accumulation does not require domain-specific request wrappers.
 */
template <typename T, typename Acc, typename Step>
Acc fold_vector(const std::vector<T> &values, Acc seed, Step step) {
  return detail::foldVectorRecursive<T, Acc, Step>(values, 0, seed, step);
}

/**
 * @brief Keeps vector values that satisfy a predicate.
 *
 * @signature template <typename T, typename Predicate> std::vector<T> filter_vector(const std::vector<T> &values, Predicate predicate)
 *
 * User Story: As selectors and ECS queries, I need reusable filtering so
 * feature code can compose predicates instead of hand-writing branches.
 */
template <typename T, typename Predicate>
std::vector<T> filter_vector(const std::vector<T> &values,
                             Predicate predicate) {
  return detail::filterVectorRecursive<T, Predicate>(values, 0, predicate,
                                                     std::vector<T>());
}

/**
 * @brief Finds the first vector value that satisfies a predicate.
 *
 * @signature template <typename T, typename Predicate> Maybe<T> find_vector(const std::vector<T> &values, Predicate predicate)
 *
 * User Story: As lookup code, I need Maybe-returning search so absence remains
 * explicit and no fallback sentinel is required.
 */
template <typename T, typename Predicate>
Maybe<T> find_vector(const std::vector<T> &values, Predicate predicate) {
  return detail::findVectorRecursive<T, Predicate>(values, 0, predicate);
}

/**
 * @brief Returns true when a vector contains a value by equality.
 *
 * @signature template <typename T> bool contains_value(const std::vector<T> &values, const T &expected)
 *
 * User Story: As predicate code, I need value membership as a neutral helper
 * instead of duplicating index loops across domains.
 */
template <typename T>
bool contains_value(const std::vector<T> &values, const T &expected) {
  return detail::vectorContainsKey<T>(values, expected, 0);
}

/**
 * @brief Keeps the first value for each projected key.
 *
 * @signature template <typename T, typename Projection> std::vector<T> unique_by(const std::vector<T> &values, Projection projection)
 *
 * User Story: As derived-data code, I need uniqueness by a reusable projection
 * so selectors and ECS queries avoid one-off dedupe helpers.
 */
template <typename T, typename Projection>
std::vector<T> unique_by(const std::vector<T> &values, Projection projection) {
  typedef typename std::decay<decltype(projection(std::declval<const T &>()))>::
      type Key;
  return detail::uniqueByRecursive<T, Projection, Key>(
      values, 0, projection, std::vector<Key>(), std::vector<T>());
}

/**
 * @brief Maps each vector value through a Maybe-returning function.
 *
 * @signature template <typename T, typename Func> auto traverse_maybe(const std::vector<T> &values, Func fn) -> Maybe<std::vector<decltype(fn(std::declval<const T &>()).value)>>
 *
 * User Story: As JSON, ECS, and adapter code, I need all-or-nothing traversal
 * so incomplete records short-circuit without hidden defaults.
 */
template <typename T, typename Func>
auto traverse_maybe(const std::vector<T> &values, Func fn)
    -> Maybe<std::vector<decltype(fn(std::declval<const T &>()).value)>> {
  typedef decltype(fn(std::declval<const T &>()).value) U;
  return detail::traverseMaybeRecursive<T, Func, U>(values, 0, fn,
                                                   std::vector<U>());
}

/**
 * @brief Converts a vector of Maybe values into a Maybe vector.
 *
 * @signature template <typename T> Maybe<std::vector<T>> sequence_maybe(const std::vector<Maybe<T>> &values)
 *
 * User Story: As composition code, I need to collect optional values only when
 * every element exists so validation remains explicit.
 */
template <typename T>
Maybe<std::vector<T>> sequence_maybe(const std::vector<Maybe<T>> &values) {
  return traverse_maybe(values, [](const Maybe<T> &value) { return value; });
}

/**
 * @brief Folds a vector through an Either-returning accumulator step.
 *
 * @signature template <typename E, typename T, typename Acc, typename Step> Either<E, Acc> fold_either(const std::vector<T> &values, Acc seed, Step step)
 *
 * User Story: As validation and reducer-boundary code, I need accumulation to
 * stop on the first error while preserving successful accumulator values.
 */
template <typename E, typename T, typename Acc, typename Step>
Either<E, Acc> fold_either(const std::vector<T> &values, Acc seed, Step step) {
  return detail::foldEitherRecursive<E, T, Acc, Step>(values, 0, seed, step);
}

/**
 * @brief Combines two Maybe values when both are present.
 *
 * @signature template <typename A, typename B, typename Combine> auto lift2(const Maybe<A> &a, const Maybe<B> &b, Combine combine) -> Maybe<decltype(combine(a.value, b.value))>
 *
 * User Story: As data assembly code, I need small optional records to compose
 * without nested matches or fallback values.
 */
template <typename A, typename B, typename Combine>
auto lift2(const Maybe<A> &a, const Maybe<B> &b, Combine combine)
    -> Maybe<decltype(combine(a.value, b.value))> {
  typedef decltype(combine(a.value, b.value)) R;
  return (a.hasValue && b.hasValue) ? just(combine(a.value, b.value))
                                    : nothing<R>();
}

/**
 * @brief Combines three Maybe values when all are present.
 *
 * @signature template <typename A, typename B, typename C, typename Combine> auto lift3(const Maybe<A> &a, const Maybe<B> &b, const Maybe<C> &c, Combine combine) -> Maybe<decltype(combine(a.value, b.value, c.value))>
 *
 * User Story: As JSON and ECS seed code, I need small required-field groups to
 * assemble through one reusable optional combinator.
 */
template <typename A, typename B, typename C, typename Combine>
auto lift3(const Maybe<A> &a, const Maybe<B> &b, const Maybe<C> &c,
           Combine combine) -> Maybe<decltype(combine(a.value, b.value, c.value))> {
  typedef decltype(combine(a.value, b.value, c.value)) R;
  return (a.hasValue && b.hasValue && c.hasValue)
             ? just(combine(a.value, b.value, c.value))
             : nothing<R>();
}


/**
 * @brief Maps a function across the populated branch of a Maybe.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename Func> auto fmap(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))>
 * 
 * User Story: As optional transformations, I need fmap on Maybe so values can
 * be transformed without unwrapping and rewrapping by hand.
 */
template <typename T, typename Func>
auto fmap(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> {
  typedef decltype(f(m.value)) U;
  return m.hasValue ? Maybe<U>{true, f(m.value)} : Maybe<U>{false, U{}};
}

/**
 * @brief Maps a function across the success branch of an Either.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T, typename Func> auto fmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))>
 * 
 * User Story: As result transformations, I need fmap on Either so success
 * values can be transformed while preserving failures unchanged.
 */
template <typename E, typename T, typename Func>
auto fmap(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))> {
  typedef decltype(f(e.right)) U;
  return e.isLeft ? Either<E, U>{true, e.left, U{}}
                  : Either<E, U>{false, E{}, f(e.right)};
}

/**
 * @brief Maps a function across every element in a vector.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature namespace detail
 * 
 * User Story: As collection transformations, I need fmap on vectors so
 * element-wise mapping follows the same functional style as Maybe and Either.
 */
namespace detail {
template <typename T, typename Func, typename U>
std::vector<U> fmapVectorRecursive(const std::vector<T> &vec, Func f,
                                   size_t index, std::vector<U> result) {
  return index == vec.size()
             ? result
             : (result.push_back(f(vec[index])),
                fmapVectorRecursive<T, Func, U>(vec, f, index + 1,
                                                std::move(result)));
}
} // namespace detail

template <typename T, typename Func>
auto fmap(const std::vector<T> &vec, Func f)
    -> std::vector<decltype(f(std::declval<const T &>()))> {
  typedef decltype(f(std::declval<const T &>())) U;
  std::vector<U> result;
  result.reserve(vec.size());
  return detail::fmapVectorRecursive<T, Func, U>(vec, f, 0, std::move(result));
}

/**
 * @brief Grid position payload used by SDK functional grid traversals.
 *
 * @signature struct GridIndex { size_t Row; size_t Column; }
 *
 * User Story: As SDK users mapping grid-shaped data, I need a neutral index
 * value that does not depend on ECS, RTK, or project code.
 */
struct GridIndex {
  size_t Row;
  size_t Column;
};

namespace detail {
template <typename IndexedCollection, typename Acc, typename Step>
Acc foldIndexedRecursive(const IndexedCollection &values, size_t count,
                         size_t index, Acc acc, Step step) {
  return index >= count
             ? acc
             : foldIndexedRecursive<IndexedCollection, Acc, Step>(
                   values, count, index + 1, step(acc, values[index]), step);
}

template <typename IndexedCollection, typename Predicate, typename Value>
Maybe<Value> findIndexedRecursive(const IndexedCollection &values, size_t count,
                                  size_t index, Predicate predicate) {
  return index >= count
             ? nothing<Value>()
             : (predicate(values[index])
                    ? just<Value>(values[index])
                    : findIndexedRecursive<IndexedCollection, Predicate, Value>(
                          values, count, index + 1, predicate));
}

template <typename Output, typename Map>
std::vector<Output> mapGridRecursive(size_t rows, size_t columns, size_t index,
                                     Map map, std::vector<Output> result) {
  return index >= rows * columns
             ? result
             : (result.push_back(
                    map(GridIndex{index / columns, index % columns})),
                mapGridRecursive<Output, Map>(rows, columns, index + 1, map,
                                              std::move(result)));
}
} // namespace detail

/**
 * @brief Folds any indexed collection through a pure accumulator step.
 *
 * @signature template <typename IndexedCollection, typename Acc, typename Step> Acc fold_indexed(const IndexedCollection &values, size_t count, Acc seed, Step step)
 *
 * User Story: As Unreal and SDK integration code, I need TArray-style indexed
 * collections to compose through the SDK FP core without local loops.
 */
template <typename IndexedCollection, typename Acc, typename Step>
Acc fold_indexed(const IndexedCollection &values, size_t count, Acc seed,
                 Step step) {
  return detail::foldIndexedRecursive<IndexedCollection, Acc, Step>(
      values, count, 0, seed, step);
}

/**
 * @brief Runs one effect for each indexed collection value.
 *
 * @signature template <typename IndexedCollection, typename Effect> void for_each_indexed(const IndexedCollection &values, size_t count, Effect effect)
 *
 * User Story: As boundary code, I need effectful indexed traversal to remain
 * centralized in the SDK functional core instead of feature-local loops.
 */
template <typename IndexedCollection, typename Effect>
void for_each_indexed(const IndexedCollection &values, size_t count,
                      Effect effect) {
  fold_indexed<IndexedCollection, int>(
      values, count, 0,
      [effect](const int &visited, const typename std::decay<
                                      decltype(std::declval<
                                                   const IndexedCollection &>()
                                                   [0])>::type &value) {
        effect(value);
        return visited + 1;
      });
}

/**
 * @brief Finds the first indexed collection value that satisfies a predicate.
 *
 * @signature template <typename IndexedCollection, typename Predicate> Maybe<Value> find_indexed(const IndexedCollection &values, size_t count, Predicate predicate)
 *
 * User Story: As lookup code, I need indexed collection searches to return
 * Maybe values without hidden sentinel fallbacks.
 */
template <typename IndexedCollection, typename Predicate>
auto find_indexed(const IndexedCollection &values, size_t count,
                  Predicate predicate)
    -> Maybe<typename std::decay<decltype(std::declval<
                                           const IndexedCollection &>()[0])>::
                 type> {
  typedef typename std::decay<decltype(std::declval<
                                       const IndexedCollection &>()[0])>::type
      Value;
  return detail::findIndexedRecursive<IndexedCollection, Predicate, Value>(
      values, count, 0, predicate);
}

/**
 * @brief Returns true when any indexed collection value satisfies a predicate.
 *
 * @signature template <typename IndexedCollection, typename Predicate> bool any_indexed(const IndexedCollection &values, size_t count, Predicate predicate)
 *
 * User Story: As predicate code, I need indexed collection checks to compose
 * through Maybe-returning SDK helpers rather than branches.
 */
template <typename IndexedCollection, typename Predicate>
bool any_indexed(const IndexedCollection &values, size_t count,
                 Predicate predicate) {
  return find_indexed<IndexedCollection, Predicate>(values, count, predicate)
      .hasValue;
}

/**
 * @brief Returns true when every indexed collection value satisfies a predicate.
 *
 * @signature template <typename IndexedCollection, typename Predicate> bool all_indexed(const IndexedCollection &values, size_t count, Predicate predicate)
 *
 * User Story: As validation code, I need universal indexed checks to compose
 * through SDK functional predicates.
 */
template <typename IndexedCollection, typename Predicate>
bool all_indexed(const IndexedCollection &values, size_t count,
                 Predicate predicate) {
  return !any_indexed<IndexedCollection>(values, count, complement(predicate));
}

/**
 * @brief Maps a rectangular grid into a vector in row-major order.
 *
 * @signature template <typename Output, typename Map> std::vector<Output> map_grid(size_t rows, size_t columns, Map map)
 *
 * User Story: As rendering and data code, I need grid mapping in the SDK FP
 * core rather than borrowing ECS grid helpers for non-ECS data.
 */
template <typename Output, typename Map>
std::vector<Output> map_grid(size_t rows, size_t columns, Map map) {
  std::vector<Output> result;
  result.reserve(rows * columns);
  return detail::mapGridRecursive<Output, Map>(rows, columns, 0, map,
                                               std::move(result));
}

/**
 * @brief Chains a Maybe-producing function onto a Maybe value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename Func> auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value))
 * 
 * User Story: As optional workflows, I need bind semantics so dependent Maybe
 * operations can short-circuit naturally on missing values.
 */
template <typename T, typename Func>
auto mbind(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return m.hasValue ? f(m.value) : decltype(f(m.value)){false, {}};
}

template <typename T, typename Func>
auto maybe_map(const Maybe<T> &m, Func f) -> Maybe<decltype(f(m.value))> {
  return fmap(m, f);
}

template <typename T, typename Func>
auto maybe_chain(const Maybe<T> &m, Func f) -> decltype(f(m.value)) {
  return mbind(m, f);
}

/**
 * @brief Chains an Either-producing function onto an Either success value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T, typename Func> auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 * 
 * User Story: As result workflows, I need bind semantics so failure branches
 * stop the pipeline while successes continue.
 */
template <typename E, typename T, typename Func>
auto ebind(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return e.isLeft ? decltype(f(e.right)){true, e.left, {}} : f(e.right);
}

/**
 * @brief Maps the success branch of an Either and preserves failures.
 *
 * @signature template <typename E, typename T, typename Func> auto either_map(const Either<E, T> &e, Func f) -> Either<E, decltype(f(e.right))>
 *
 * User Story: As a result pipeline author, I need to transform successful
 * payloads without unpacking or disturbing the error branch.
 */
template <typename E, typename T, typename Func>
auto either_map(const Either<E, T> &e, Func f)
    -> Either<E, decltype(f(e.right))> {
  return fmap(e, f);
}

/**
 * @brief Chains an Either-producing function from a successful Either.
 *
 * @signature template <typename E, typename T, typename Func> auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right))
 *
 * User Story: As a result pipeline author, I need to sequence fallible steps
 * so the first Left skips all downstream work.
 */
template <typename E, typename T, typename Func>
auto either_chain(const Either<E, T> &e, Func f) -> decltype(f(e.right)) {
  return ebind(e, f);
}

/**
 * @brief Maps the failure branch of an Either and preserves successes.
 *
 * @signature template <typename E, typename T, typename Func> auto either_map_left(const Either<E, T> &e, Func f) -> Either<decltype(f(e.left)), T>
 *
 * User Story: As a boundary author, I need to translate low-level errors into
 * domain errors without touching the successful payload path.
 */
template <typename E, typename T, typename Func>
auto either_map_left(const Either<E, T> &e, Func f)
    -> Either<decltype(f(e.left)), T> {
  typedef decltype(f(e.left)) NextError;
  std::function<Either<NextError, T>()> Cases[2] = {
      [&]() { return make_right<NextError, T>(e.right); },
      [&]() { return make_left<NextError, T>(f(e.left)); }};
  return Cases[static_cast<size_t>(e.isLeft)]();
}

/**
 * @brief Extracts a Maybe value or returns the provided default.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> T or_else(const Maybe<T> &m, const T &def)
 * 
 * User Story: As boundary code, I need a defaulting helper so Maybe values can
 * be converted into concrete values at integration points.
 */
template <typename T> T or_else(const Maybe<T> &m, const T &def) {
  return m.hasValue ? m.value : def;
}

template <typename T, typename DefaultFactory>
T maybe_or_else(const Maybe<T> &m, DefaultFactory defaultFactory) {
  return m.hasValue ? m.value : defaultFactory();
}

/**
 * @brief Extracts an Either success value or returns a default for failures.
 *
 * @signature template <typename E, typename T> T either_or_else(const Either<E, T> &e, const T &def)
 *
 * User Story: As an integration author, I need to collapse recoverable
 * failures into a concrete fallback at engine and UI boundaries.
 */
template <typename E, typename T>
T either_or_else(const Either<E, T> &e, const T &def) {
  std::function<T()> Cases[2] = {[&]() { return e.right; },
                                 [&]() { return def; }};
  return Cases[static_cast<size_t>(e.isLeft)]();
}

/**
 * @brief Pattern matches on a Maybe with Just and Nothing callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename FJust, typename FNothing> auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing) -> decltype(onJust(m.value))
 * 
 * User Story: As boundary code, I need pattern matching on Maybe so success and
 * empty branches can be handled declaratively.
 */
template <typename T, typename FJust, typename FNothing>
auto match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return m.hasValue ? onJust(m.value) : onNothing();
}

template <typename T, typename FJust, typename FNothing>
auto maybe_match(const Maybe<T> &m, FJust onJust, FNothing onNothing)
    -> decltype(onJust(m.value)) {
  return match(m, onJust, onNothing);
}

/**
 * @brief Pattern matches on an Either with error and success callbacks.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename E, typename T, typename FLeft, typename FRight> auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight) -> decltype(onRight(e.right))
 * 
 * User Story: As boundary code, I need pattern matching on Either so success
 * and failure handling stay explicit and type-safe.
 */
template <typename E, typename T, typename FLeft, typename FRight>
auto ematch(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return e.isLeft ? onLeft(e.left) : onRight(e.right);
}

template <typename E, typename T, typename FLeft, typename FRight>
auto either_match(const Either<E, T> &e, FLeft onLeft, FRight onRight)
    -> decltype(onRight(e.right)) {
  return ematch(e, onLeft, onRight);
}

/**
 * @brief Unreal container combinators for TArray/TMap using the same FP
 * semantics as the vector, Maybe, Either, indexed, and catalog primitives.
 *
 * User Story: As UE feature, RTK, and ECS code, I need Unreal-native
 * collection traversal, lookup, update, and equality helpers to live in the
 * FP core so higher domains compose neutral functions instead of owning local
 * loop, request, or factory families.
 */
template <typename Item, typename Acc, typename Step>
Acc fold_array(const TArray<Item> &values, Acc seed, Step step) {
  return fold_indexed<TArray<Item>, Acc>(
      values, static_cast<size_t>(values.Num()), seed,
      [step](const Acc &acc, const Item &item) { return step(acc, item); });
}

template <typename Item, typename Effect>
void for_each_array(const TArray<Item> &values, Effect effect) {
  for_each_indexed<TArray<Item>, Effect>(
      values, static_cast<size_t>(values.Num()), effect);
}

template <typename Item, typename Predicate>
bool all_array(const TArray<Item> &values, Predicate predicate) {
  return all_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

template <typename Item, typename Predicate>
bool any_array(const TArray<Item> &values, Predicate predicate) {
  return any_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

template <typename Item>
bool contains_value(const TArray<Item> &values, const Item &expected) {
  return any_array<Item>(
      values, [&expected](const Item &value) { return value == expected; });
}

template <typename Item, typename Predicate>
Maybe<Item> find_array(const TArray<Item> &values, Predicate predicate) {
  return find_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

template <typename Item>
TArray<Item> append_value(TArray<Item> values, const Item &value) {
  values.Add(value);
  return values;
}

template <typename Item>
TArray<Item> append_unique_value(TArray<Item> values, const Item &value) {
  values.AddUnique(value);
  return values;
}

template <typename Item>
TArray<Item> append_values(TArray<Item> values,
                           const TArray<Item> &additional_values) {
  values.Append(additional_values);
  return values;
}

template <typename Item, typename Predicate>
TArray<Item> filter_array(const TArray<Item> &values, Predicate predicate) {
  return fold_array<Item, TArray<Item>>(
      values, TArray<Item>(),
      [predicate](const TArray<Item> &acc, const Item &value) {
        return predicate(value) ? append_value<Item>(acc, value) : acc;
      });
}

template <typename Source, typename Map>
auto map_array(const TArray<Source> &values, Map map)
    -> TArray<decltype(map(std::declval<const Source &>()))> {
  typedef decltype(map(std::declval<const Source &>())) Output;
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [map](const TArray<Output> &acc, const Source &value) {
        return append_value<Output>(acc, map(value));
      });
}

template <typename Source, typename Output, typename Map>
TArray<Output> map_array(const TArray<Source> &values, Map map) {
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [map](const TArray<Output> &acc, const Source &value) {
        return append_value<Output>(acc, map(value));
      });
}

template <typename Source, typename Keep, typename Map>
auto filter_map_array(const TArray<Source> &values, Keep keep, Map map)
    -> TArray<decltype(map(std::declval<const Source &>()))> {
  typedef decltype(map(std::declval<const Source &>())) Output;
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [keep, map](const TArray<Output> &acc, const Source &value) {
        return keep(value) ? append_value<Output>(acc, map(value)) : acc;
      });
}

template <typename Source, typename Output, typename Keep, typename Map>
TArray<Output> filter_map_array(const TArray<Source> &values, Keep keep,
                                Map map) {
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [keep, map](const TArray<Output> &acc, const Source &value) {
        return keep(value) ? append_value<Output>(acc, map(value)) : acc;
      });
}

template <typename T, typename Func>
auto fmap(const TArray<T> &values, Func f)
    -> TArray<decltype(f(std::declval<const T &>()))> {
  return map_array<T, Func>(values, f);
}

template <typename Source, typename Map>
auto traverse_maybe_array(const TArray<Source> &values, Map map)
    -> Maybe<TArray<decltype(map(std::declval<const Source &>()).value)>> {
  typedef decltype(map(std::declval<const Source &>()).value) Output;
  return fold_array<Source, Maybe<TArray<Output>>>(
      values, just<TArray<Output>>(TArray<Output>()),
      [map](const Maybe<TArray<Output>> &acc, const Source &value) {
        return match(
            acc,
            [map, &value](const TArray<Output> &items) {
              return match(
                  map(value),
                  [&items](const Output &output) {
                    return just<TArray<Output>>(
                        append_value<Output>(items, output));
                  },
                  []() { return nothing<TArray<Output>>(); });
            },
            []() { return nothing<TArray<Output>>(); });
      });
}

template <typename Source, typename Output, typename Map>
Maybe<TArray<Output>> traverse_maybe_array(const TArray<Source> &values,
                                           Map map) {
  return traverse_maybe_array<Source, Map>(values, map);
}

template <typename T>
Maybe<TArray<T>> sequence_maybe_array(const TArray<Maybe<T>> &values) {
  return traverse_maybe_array(
      values, [](const Maybe<T> &value) { return value; });
}

template <typename E, typename Item, typename Acc, typename Step>
Either<E, Acc> fold_either_array(const TArray<Item> &values, Acc seed,
                                 Step step) {
  return fold_array<Item, Either<E, Acc>>(
      values, make_right<E, Acc>(seed),
      [step](const Either<E, Acc> &acc, const Item &value) {
        return ebind(acc, [&value, step](const Acc &current) {
          return step(current, value);
        });
      });
}

template <typename Item>
TArray<Item> concat_arrays(const TArray<TArray<Item>> &arrays) {
  return fold_array<TArray<Item>, TArray<Item>>(
      arrays, TArray<Item>(),
      [](const TArray<Item> &acc, const TArray<Item> &values) {
        return append_values<Item>(acc, values);
      });
}

template <typename Item>
TArray<Item> unique_array(const TArray<Item> &values) {
  return fold_array<Item, TArray<Item>>(
      values, TArray<Item>(),
      [](const TArray<Item> &acc, const Item &value) {
        return append_unique_value<Item>(acc, value);
      });
}

template <typename Acc, typename Step>
Acc fold_index_range(int32 count, Acc seed, Step step, int32 index = 0) {
  return index >= count
             ? seed
             : fold_index_range<Acc, Step>(count, step(seed, index), step,
                                           index + 1);
}

inline TArray<int32> index_range(int32 count) {
  return fold_index_range<TArray<int32>>(
      count, TArray<int32>(),
      [](const TArray<int32> &acc, int32 index) {
        return append_value<int32>(acc, index);
      });
}

template <typename Map>
auto map_index_range(int32 count, Map map)
    -> TArray<decltype(map(std::declval<int32>()))> {
  typedef decltype(map(std::declval<int32>())) Output;
  return fold_index_range<TArray<Output>>(
      count, TArray<Output>(),
      [map](const TArray<Output> &acc, int32 index) {
        return append_value<Output>(acc, map(index));
      });
}

template <typename Source, typename Map>
auto traverse_maybe_array_with_index(const TArray<Source> &values, Map map)
    -> Maybe<TArray<decltype(map(std::declval<const Source &>(),
                                  std::declval<int32>()).value)>> {
  typedef decltype(map(std::declval<const Source &>(),
                       std::declval<int32>()).value) Output;
  return fold_index_range<Maybe<TArray<Output>>>(
      values.Num(), just<TArray<Output>>(TArray<Output>()),
      [&values, map](const Maybe<TArray<Output>> &acc, int32 index) {
        return match(
            acc,
            [&values, map, index](const TArray<Output> &items) {
              return match(
                  map(values[index], index),
                  [&items](const Output &output) {
                    return just<TArray<Output>>(
                        append_value<Output>(items, output));
                  },
                  []() { return nothing<TArray<Output>>(); });
            },
            []() { return nothing<TArray<Output>>(); });
      });
}

template <typename Source, typename Output, typename Map>
Maybe<TArray<Output>>
traverse_maybe_array_with_index(const TArray<Source> &values, Map map) {
  return fold_index_range<Maybe<TArray<Output>>>(
      values.Num(), just<TArray<Output>>(TArray<Output>()),
      [&values, map](const Maybe<TArray<Output>> &acc, int32 index) {
        return match(
            acc,
            [&values, map, index](const TArray<Output> &items) {
              return match(
                  map(values[index], index),
                  [&items](const Output &output) {
                    return just<TArray<Output>>(
                        append_value<Output>(items, output));
                  },
                  []() { return nothing<TArray<Output>>(); });
            },
            []() { return nothing<TArray<Output>>(); });
      });
}

template <typename Acc, typename Step>
Acc fold_grid_range(size_t rows, size_t columns, Acc seed, Step step) {
  return fold_index_range<Acc>(
      static_cast<int32>(rows), seed,
      [columns, step](const Acc &row_acc, int32 row) {
        return fold_index_range<Acc>(
            static_cast<int32>(columns), row_acc,
            [row, step](const Acc &column_acc, int32 column) {
              return step(column_acc,
                          GridIndex{static_cast<size_t>(row),
                                    static_cast<size_t>(column)});
            });
      });
}

template <typename Output, typename Map>
TArray<Output> map_grid_array(size_t rows, size_t columns, Map map) {
  return fold_grid_range<TArray<Output>>(
      rows, columns, TArray<Output>(),
      [map](const TArray<Output> &acc, const GridIndex &index) {
        return append_value<Output>(acc, map(index));
      });
}

template <typename Key, typename Value>
TArray<Key> map_keys(const TMap<Key, Value> &values) {
  TArray<Key> keys;
  values.GetKeys(keys);
  return keys;
}

template <typename Key, typename Value>
TArray<Key> append_map_keys(TArray<Key> values,
                            const TMap<Key, Value> &map) {
  return append_values<Key>(values, map_keys<Key, Value>(map));
}

template <typename Key, typename Value>
Maybe<const Value *> find_map_value_ptr(const TMap<Key, Value> &values,
                                        const Key &key) {
  const Value *found = values.Find(key);
  return found ? just<const Value *>(found) : nothing<const Value *>();
}

template <typename Key, typename Value>
Maybe<Value> find_map_value(const TMap<Key, Value> &values, const Key &key) {
  return match(
      find_map_value_ptr<Key, Value>(values, key),
      [](const Value *found) { return just<Value>(*found); },
      []() { return nothing<Value>(); });
}

template <typename Key, typename Value>
Value map_value_or(const TMap<Key, Value> &values, const Key &key,
                   const Value &fallback) {
  return or_else(find_map_value<Key, Value>(values, key), fallback);
}

template <typename Key, typename Value, typename Transform>
TMap<Key, Value> update_map_value_when_present(TMap<Key, Value> values,
                                               const Key &key,
                                               Transform transform) {
  return match(
      find_map_value<Key, Value>(values, key),
      [values, key, transform](const Value &found) mutable {
        values.Add(key, transform(found));
        return values;
      },
      [values]() { return values; });
}

template <typename Key, typename Value, typename Transform>
TMap<Key, Value> upsert_map_value(TMap<Key, Value> values, const Key &key,
                                  const Value &fallback,
                                  Transform transform) {
  values.Add(key, transform(map_value_or<Key, Value>(values, key, fallback)));
  return values;
}

template <typename Key, typename Value>
std::function<bool(const Key &)> map_has_key(const TMap<Key, Value> &values) {
  return [&values](const Key &key) { return values.Contains(key); };
}

template <typename Key, typename Item>
bool map_array_contains(const TMap<Key, TArray<Item>> &values,
                        const Key &key, const Item &item) {
  return match(
      find_map_value_ptr<Key, TArray<Item>>(values, key),
      [&item](const TArray<Item> *items) {
        return contains_value<Item>(*items, item);
      },
      []() { return false; });
}

template <typename Key, typename Value, typename Equals>
bool map_values_equal(const TMap<Key, Value> &left,
                      const TMap<Key, Value> &right, Equals equals) {
  const TArray<Key> keys = map_keys<Key, Value>(left);
  return left.Num() == right.Num() &&
         all_array<Key>(
             keys, [&left, &right, equals](const Key &key) {
               return match(
                   find_map_value_ptr<Key, Value>(right, key),
                   [&left, &key, equals](const Value *right_value) {
                     const Value *left_value = left.Find(key);
                     return left_value && equals(*left_value, *right_value);
                   },
                   []() { return false; });
             });
}

template <typename Key, typename Value, typename ErrorMessage>
auto require_map_key(const TMap<Key, Value> &values,
                     ErrorMessage error_message)
    -> std::function<Either<decltype(error_message(
                         std::declval<const Key &>())), bool>(const Key &)> {
  typedef decltype(error_message(std::declval<const Key &>())) Error;
  const std::function<bool(const Key &)> exists =
      map_has_key<Key, Value>(values);
  return [exists, error_message](const Key &key) {
    return exists(key) ? make_right<Error, bool>(true)
                       : make_left<Error, bool>(error_message(key));
  };
}

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
 * @brief Creates an empty validation pipeline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename E = std::string> ValidationPipeline<T, E> validationPipeline()
 * 
 * User Story: As validation flows, I need a pipeline entry point so validators
 * can be declared and composed incrementally.
 */
template <typename T, typename E = std::string>
ValidationPipeline<T, E> validationPipeline() {
  return ValidationPipeline<T, E>{{}};
}

/**
 * @brief Appends a validator to the pipeline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename E, typename Func> ValidationPipeline<T, E> addValidation(ValidationPipeline<T, E> Pipeline, Func Validator)
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
 * @brief Supports pipe-style validation assembly with free functions.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename E, typename Func> ValidationPipeline<T, E> operator|(ValidationPipeline<T, E> Pipeline, Func Validator)
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
template <typename T, typename E>
Either<E, T>
runValidationRecursive(const std::vector<std::function<Either<E, T>(T)>> &Steps,
                       size_t Index, T Current);

template <typename T, typename E>
Either<E, T>
runValidationStep(const std::vector<std::function<Either<E, T>(T)>> &Steps,
                  size_t Index, T Current) {
  Either<E, T> Result = Steps[Index](Current);
  return Result.isLeft ? Result
                       : runValidationRecursive<T, E>(Steps, Index + 1,
                                                      Result.right);
}

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
 * @brief Runs validators in order and stops on the first error.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename E> Either<E, T> runValidation(const ValidationPipeline<T, E> &Pipeline, T Value)
 * 
 * User Story: As validation flows, I need short-circuit execution so failing
 * input stops at the first invalid step.
 */
template <typename T, typename E>
Either<E, T> runValidation(const ValidationPipeline<T, E> &Pipeline, T Value) {
  return detail::runValidationRecursive<T, E>(Pipeline.Validators, 0,
                                              std::move(Value));
}

/**
 * @brief 14. ConfigBuilder (Functional Configuration Builder) A data-first builder for creating immutable configuration objects using functional composition. Usage: auto builder = configBuilder<MyConfig>(); builder = setMember(builder, &MyConfig::name, std::string("MyApp")); builder = setMember(builder, &MyConfig::port, 8080); auto config = buildConfig(builder);
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Config> struct ConfigBuilder
 * 
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <typename Config> struct ConfigBuilder {
  std::vector<std::function<void(Config &)>> Setters;
};

/**
 * @brief Creates an empty functional configuration builder.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Config> ConfigBuilder<Config> configBuilder()
 * 
 * User Story: As config assembly flows, I need a builder entry point so
 * immutable config values can be constructed declaratively.
 */
template <typename Config> ConfigBuilder<Config> configBuilder() {
  return ConfigBuilder<Config>{{}};
}

/**
 * @brief Adds an explicit mutating transform to the eventual config value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Config, typename Func> ConfigBuilder<Config> with(ConfigBuilder<Config> Builder, Func Setter)
 * 
 * User Story: As config assembly flows, I need queued setters so immutable
 * config objects can be built through composable transforms.
 */
template <typename Config, typename Func>
ConfigBuilder<Config> with(ConfigBuilder<Config> Builder, Func Setter) {
  std::function<void(Config &)> WrappedSetter = Setter;
  Builder.Setters.push_back(WrappedSetter);
  return Builder;
}

/**
 * @brief Assigns a concrete member through a pointer-to-member.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Config, typename T> ConfigBuilder<Config> setMember(ConfigBuilder<Config> Builder, T Config::*Member, T Value)
 * 
 * User Story: As config assembly flows, I need member assignment helpers so
 * config values can be declared without repetitive boilerplate.
 */
template <typename Config, typename T>
ConfigBuilder<Config> setMember(ConfigBuilder<Config> Builder,
                                T Config::*Member, T Value) {
  return with(std::move(Builder),
              [Member, Value](Config &ConfigValue) mutable {
                ConfigValue.*Member = std::move(Value);
              });
}

/**
 * @brief Delegates string-keyed assignment to config types that expose `set`.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Config, typename T> ConfigBuilder<Config> set(ConfigBuilder<Config> Builder, const std::string &Key, T Value)
 * 
 * User Story: As config assembly flows, I need key-based setters so dynamic
 * config types can participate in the same builder pattern.
 */
template <typename Config, typename T>
ConfigBuilder<Config> set(ConfigBuilder<Config> Builder, const std::string &Key,
                          T Value) {
  return with(std::move(Builder), [Key, Value](Config &ConfigValue) mutable {
    ConfigValue.set(Key, std::move(Value));
  });
}

/**
 * @brief Materializes the configured value by replaying all queued setters.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature namespace detail
 * 
 * User Story: As config assembly flows, I need a final build step so queued
 * transforms can produce one immutable config value.
 */
namespace detail {
template <typename Config>
Config applyConfigSettersRecursive(
    const std::vector<std::function<void(Config &)>> &Setters, size_t Index,
    Config ConfigValue) {
  return Index == Setters.size()
             ? ConfigValue
             : (Setters[Index](ConfigValue),
                applyConfigSettersRecursive(Setters, Index + 1,
                                            std::move(ConfigValue)));
}
} // namespace detail

template <typename Config> Config buildConfig(const ConfigBuilder<Config> &Builder) {
  return detail::applyConfigSettersRecursive(Builder.Setters, 0, Config{});
}

namespace detail {
template <typename T> T failWithMessage(const std::string &Message);
} // namespace detail

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
 * @brief Builds a successful test result with an attached value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static TestResult<T> Success(T value, std::string message = "")
 * 
 * User Story: As functional tests, I need a success factory so assertions can
   * return values and metadata through one result type.
 */
  static TestResult<T> Success(T value, std::string message = "") {
    return TestResult<T>{true, std::move(value), std::move(message), {}};
  }

/**
 * @brief Builds a failed test result with a message.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static TestResult<T> Failure(std::string message)
 * 
 * User Story: As functional tests, I need a failure factory so assertion
   * failures can be reported without exceptions or ad hoc flags.
 */
  static TestResult<T> Failure(std::string message) {
    return TestResult<T>{false, T{}, std::move(message), {}};
  }

/**
 * @brief Attaches a string detail pair to the result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature TestResult &withDetail(const std::string &key, const std::string &val)
 * 
 * User Story: As functional tests, I need structured detail fields so
   * failures and successes can carry extra diagnostic context.
 */
  TestResult &withDetail(const std::string &key, const std::string &val) {
    details[key] = val;
    return *this;
  }

/**
 * @brief Reports whether the result represents success.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature bool isSuccessful() const
 * 
 * User Story: As functional tests, I need a direct success check so calling
   * code can branch without inspecting raw fields.
 */
  bool isSuccessful() const { return bSuccess; }

/**
 * @brief Returns the value as a Maybe when the test succeeded.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature Maybe<T> TryGetValue() const
 * 
 * User Story: As functional tests, I need a non-throwing accessor so
   * no-exception builds can read successful values safely.
 */
  Maybe<T> TryGetValue() const { return bSuccess ? just(value) : nothing<T>(); }

/**
 * @brief Returns the value or fails fast when the result is unsuccessful.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature T getValue() const
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
 * @brief Builds a successful void test result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static TestResult<void> Success(std::string message = "")
 * 
 * User Story: As functional tests, I need a void success factory so
   * side-effect-only assertions can still return structured outcomes.
 */
  static TestResult<void> Success(std::string message = "") {
    return TestResult<void>{true, std::move(message), {}};
  }

/**
 * @brief Builds a failed void test result with a message.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static TestResult<void> Failure(std::string message)
 * 
 * User Story: As functional tests, I need a void failure factory so
   * assertion failures can be reported even when no value is returned.
 */
  static TestResult<void> Failure(std::string message) {
    return TestResult<void>{false, std::move(message), {}};
  }

/**
 * @brief Attaches a string detail pair to the void result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature TestResult &withDetail(const std::string &key, const std::string &val)
 * 
 * User Story: As functional tests, I need structured detail fields so
   * void assertions can still surface diagnostic metadata.
 */
  TestResult &withDetail(const std::string &key, const std::string &val) {
    details[key] = val;
    return *this;
  }

/**
 * @brief Reports whether the void result represents success.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature bool isSuccessful() const
 * 
 * User Story: As functional tests, I need a direct success check so callers
   * can branch on pass or fail without inspecting raw fields.
 */
  bool isSuccessful() const { return bSuccess; }
};

/**
 * @brief 16. AsyncResult (Functional Async Result Handling) A type for handling async operations that can succeed or fail, with support for chaining and error handling. Safe for async callbacks via shared state. Usage: auto result = AsyncResult<int>::create([]( std::function<void(int)> resolve, std::function<void(std::string)> reject) { // async operation }); result.then([](int value) { // success }).catch_([](std::string error) { // failure }).execute();
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature namespace detail
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

namespace detail {
template <typename T> T failWithMessage(const std::string &Message) {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
  throw std::runtime_error(Message);
#else
  std::abort();
#endif
}
} // namespace detail

template <typename T> struct AsyncResult;

template <typename T>
AsyncResult<T>
createAsyncResult(std::function<void(std::function<void(T)>,
                                     std::function<void(std::string)>)>
                      executor);

template <typename T, typename Handler>
const AsyncResult<T> &thenAsync(const AsyncResult<T> &result,
                                Handler handler);

template <typename T, typename Handler>
const AsyncResult<T> &catchAsync(const AsyncResult<T> &result,
                                 Handler handler);

template <typename T> void executeAsync(const AsyncResult<T> &result);

inline AsyncResult<void>
createAsyncResult(std::function<void(std::function<void()>,
                                     std::function<void(std::string)>)>
                      executor);

template <typename Handler>
inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result,
                                          Handler handler);

template <typename Handler>
inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result,
                                           Handler handler);

inline void executeAsync(const AsyncResult<void> &result);

template <typename T> struct AsyncResult {
  struct State {
    std::function<void(std::function<void(T)>,
                       std::function<void(std::string)>)>
        executor;
    std::vector<std::function<void(T)>> successHandlers;
    std::vector<std::function<void(std::string)>> errorHandlers;
  };
  std::shared_ptr<State> state = std::make_shared<State>();

/**
 * @brief Builds an async result from an executor callback.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static AsyncResult<T> create(std::function<void(std::function<void(T)>, std::function<void(std::string)>)> executor)
 * 
 * User Story: As async composition code, I need a factory that captures an
   * executor so asynchronous work can be chained through one result type.
 */
  static AsyncResult<T>
  create(std::function<void(std::function<void(T)>,
                            std::function<void(std::string)>)>
             executor) {
    return createAsyncResult<T>(std::move(executor));
  }

/**
 * @brief Registers a success handler on the async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature const AsyncResult<T> &then(std::function<void(T)> handler) const
 * 
 * User Story: As async composition code, I need success callbacks so resolved
   * values can trigger follow-up behavior without blocking.
 */
  const AsyncResult<T> &then(std::function<void(T)> handler) const {
    return thenAsync(*this, std::move(handler));
  }

/**
 * @brief Registers an error handler on the async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature const AsyncResult<T> &catch_(std::function<void(std::string)> handler) const
 * 
 * User Story: As async composition code, I need error callbacks so rejected
   * work can surface failures through the same fluent API.
 */
  const AsyncResult<T> &catch_(std::function<void(std::string)> handler) const {
    return catchAsync(*this, std::move(handler));
  }

/**
 * @brief Executes the stored async operation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature void execute() const
 * 
 * User Story: As async composition code, I need an explicit execute step so
   * async pipelines run only when the caller is ready to trigger them.
 */
  void execute() const { executeAsync(*this); }
};

/**
 * @brief Specialization for void
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <> struct AsyncResult<void>
 * 
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

template <> struct AsyncResult<void> {
  struct State {
    std::function<void(std::function<void()>, std::function<void(std::string)>)>
        executor;
    std::vector<std::function<void()>> successHandlers;
    std::vector<std::function<void(std::string)>> errorHandlers;
  };
  std::shared_ptr<State> state = std::make_shared<State>();

/**
 * @brief Builds a void async result from an executor callback.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static AsyncResult<void> create(std::function<void(std::function<void()>, std::function<void(std::string)>)> executor)
 * 
 * User Story: As async composition code, I need a void factory so fire-and-
   * signal tasks can share the same chaining surface as valued tasks.
 */
  static AsyncResult<void>
  create(std::function<void(std::function<void()>,
                            std::function<void(std::string)>)>
             executor) {
    return createAsyncResult(std::move(executor));
  }

/**
 * @brief Registers a success handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature const AsyncResult<void> &then(std::function<void()> handler) const
 * 
 * User Story: As async composition code, I need success callbacks so
   * completion-only tasks can notify later stages without return values.
 */
  const AsyncResult<void> &then(std::function<void()> handler) const {
    return thenAsync(*this, std::move(handler));
  }

/**
 * @brief Registers an error handler on the void async result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature const AsyncResult<void> & catch_(std::function<void(std::string)> handler) const
 * 
 * User Story: As async composition code, I need error callbacks so void
   * tasks can surface failures through the same fluent interface.
 */
  const AsyncResult<void> &
  catch_(std::function<void(std::string)> handler) const {
    return catchAsync(*this, std::move(handler));
  }

/**
 * @brief Executes the stored void async operation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature void execute() const
 * 
 * User Story: As async composition code, I need an explicit execute step so
   * completion-only async pipelines run on demand.
 */
  void execute() const { executeAsync(*this); }
};

namespace detail {
template <typename T>
void invokeSuccessHandlersRecursive(
    const std::vector<std::function<void(T)>> &Handlers, size_t Index,
    T Value) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Value),
         invokeSuccessHandlersRecursive<T>(Handlers, Index + 1, Value));
}

inline void
invokeVoidHandlersRecursive(const std::vector<std::function<void()>> &Handlers,
                            size_t Index) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](), invokeVoidHandlersRecursive(Handlers, Index + 1));
}

inline void invokeErrorHandlersRecursive(
    const std::vector<std::function<void(std::string)>> &Handlers,
    size_t Index, const std::string &Error) {
  Index == Handlers.size()
      ? void()
      : (Handlers[Index](Error),
         invokeErrorHandlersRecursive(Handlers, Index + 1, Error));
}

template <typename T>
void runAsyncExecutor(
    const std::shared_ptr<typename AsyncResult<T>::State> &CapturedState) {
  CapturedState->executor(
      [CapturedState](T Value) {
        invokeSuccessHandlersRecursive<T>(CapturedState->successHandlers, 0,
                                          Value);
      },
      [CapturedState](std::string Error) {
        invokeErrorHandlersRecursive(CapturedState->errorHandlers, 0, Error);
      });
}

inline void runAsyncExecutor(
    const std::shared_ptr<typename AsyncResult<void>::State> &CapturedState) {
  CapturedState->executor(
      [CapturedState]() {
        invokeVoidHandlersRecursive(CapturedState->successHandlers, 0);
      },
      [CapturedState](std::string Error) {
        invokeErrorHandlersRecursive(CapturedState->errorHandlers, 0, Error);
      });
}
} // namespace detail

template <typename T>
AsyncResult<T>
createAsyncResult(std::function<void(std::function<void(T)>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<T> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

template <typename T, typename Handler>
const AsyncResult<T> &thenAsync(const AsyncResult<T> &result,
                                Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void(T)>(std::move(handler))),
                result)
             : result;
}

template <typename T, typename Handler>
const AsyncResult<T> &catchAsync(const AsyncResult<T> &result,
                                 Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

template <typename T> void executeAsync(const AsyncResult<T> &result) {
  const std::shared_ptr<typename AsyncResult<T>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor<T>(CapturedState)
      : void();
}

inline AsyncResult<void>
createAsyncResult(std::function<void(std::function<void()>,
                                     std::function<void(std::string)>)>
                      executor) {
  AsyncResult<void> Result;
  Result.state->executor = std::move(executor);
  return Result;
}

template <typename Handler>
inline const AsyncResult<void> &thenAsync(const AsyncResult<void> &result,
                                          Handler handler) {
  return result.state
             ? (result.state->successHandlers.push_back(
                    std::function<void()>(std::move(handler))),
                result)
             : result;
}

template <typename Handler>
inline const AsyncResult<void> &catchAsync(const AsyncResult<void> &result,
                                           Handler handler) {
  return result.state
             ? (result.state->errorHandlers.push_back(
                    std::function<void(std::string)>(std::move(handler))),
                result)
             : result;
}

inline void executeAsync(const AsyncResult<void> &result) {
  const std::shared_ptr<typename AsyncResult<void>::State> CapturedState =
      result.state;
  (CapturedState && CapturedState->executor)
      ? detail::runAsyncExecutor(CapturedState)
      : void();
}

/**
 * @brief 17. HttpResult (Functional Http Request Wrapper)
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature typedef std::int32_t HttpStatusCode
 * 
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

typedef std::int32_t HttpStatusCode;

template <typename T> struct HttpResult {
  bool bSuccess;
  HttpStatusCode ResponseCode;
  T data;
  std::string error;

/**
 * @brief Builds a successful HTTP result wrapper.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static HttpResult<T> Success(T d, HttpStatusCode code = 200)
 * 
 * User Story: As HTTP adapter code, I need a success factory so decoded
   * payloads carry both data and transport status through one value.
 */
  static HttpResult<T> Success(T d, HttpStatusCode code = 200) {
    return HttpResult<T>{true, code, std::move(d), ""};
  }

/**
 * @brief Builds a failed HTTP result wrapper.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature static HttpResult<T> Failure(std::string e, HttpStatusCode code = 0)
 * 
 * User Story: As HTTP adapter code, I need a failure factory so transport or
   * decoding errors can move through the same result channel as successes.
 */
  static HttpResult<T> Failure(std::string e, HttpStatusCode code = 0) {
    return HttpResult<T>{false, code, T{}, std::move(e)};
  }
};

/**
 * @brief 18. AsyncChain (Helpers for chaining AsyncResults)
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature namespace AsyncChain
 * 
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

namespace AsyncChain {
/**
 * @brief Chains one AsyncResult into another async-producing transformation.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename U, typename F> auto then(const AsyncResult<T> &res, F f) -> AsyncResult<U>
 * 
 * User Story: As async thunk composition, I need async chaining so one async
 * result can feed into the next without nested callback plumbing.
 */
template <typename T, typename U, typename F>
auto then(const AsyncResult<T> &res, F f) -> AsyncResult<U> {
  return createAsyncResult<U>(
      [res, f](std::function<void(U)> resolve,
               std::function<void(std::string)> reject) {
        AsyncResult<T> Source = res;
        thenAsync(Source, [f, resolve, reject](T val) {
          AsyncResult<U> Next = f(val);
          thenAsync(Next, resolve);
          catchAsync(Next, reject);
          executeAsync(Next);
        });
        catchAsync(Source, reject);
        executeAsync(Source);
      });
}
} // namespace AsyncChain

/**
 * @brief 19. Dispatcher (Dictionary-Based Typed Dispatch) A lookup table mapping keys to handler functions. Returns Maybe<Result> from dispatch — just(handler()) if the key exists, nothing<Result>() if not. Construction: use createDispatcher<Key, Result>() with a vector of {key, handler} pairs. Dispatch:     use the dispatch() free function. Usage: auto d = func::createDispatcher<FString, int>({ {TEXT("a"), []() { return 1; }}, {TEXT("b"), []() { return 2; }}, }); auto result = func::dispatch(d, TEXT("a")); // just(1) auto miss   = func::dispatch(d, TEXT("z")); // nothing
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Key, typename Result> struct Dispatcher
 * 
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */

template <typename Key, typename Result> struct Dispatcher {
  std::unordered_map<Key, std::function<Result()>> table;
};

namespace detail {
template <typename Key, typename Result>
Dispatcher<Key, Result>
createDispatcherRecursive(
    const std::vector<std::pair<Key, std::function<Result()>>> &Entries,
    size_t Index, Dispatcher<Key, Result> Current) {
  return Index == Entries.size()
             ? Current
             : (Current.table[Entries[Index].first] = Entries[Index].second,
                createDispatcherRecursive<Key, Result>(Entries, Index + 1,
                                                       std::move(Current)));
}

template <typename Key, typename Result>
std::vector<Key> dispatcherKeysRecursive(
    typename std::unordered_map<Key, std::function<Result()>>::const_iterator It,
    typename std::unordered_map<Key, std::function<Result()>>::const_iterator End,
    std::vector<Key> Current) {
  return It == End
             ? Current
             : (Current.push_back(It->first),
                dispatcherKeysRecursive<Key, Result>(++It, End,
                                                     std::move(Current)));
}
} // namespace detail

/**
 * @brief Builds a dispatcher table from key-to-handler entries.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Key, typename Result> Dispatcher<Key, Result> createDispatcher( std::vector<std::pair<Key, std::function<Result()>>> entries)
 * 
 * User Story: As keyed dispatch flows, I need a typed dispatcher table so
 * string or enum keys can resolve handlers declaratively.
 */
template <typename Key, typename Result>
Dispatcher<Key, Result> createDispatcher(
    std::vector<std::pair<Key, std::function<Result()>>> entries) {
  return detail::createDispatcherRecursive<Key, Result>(entries, 0,
                                                        Dispatcher<Key, Result>{});
}

/**
 * @brief Looks up and invokes a handler by key when one exists.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Key, typename Result> Maybe<Result> dispatch(const Dispatcher<Key, Result> &d, const Key &key)
 * 
 * User Story: As keyed dispatch flows, I need dispatch to return Maybe so
 * missing handlers do not require exceptions or sentinels.
 */
template <typename Key, typename Result>
Maybe<Result> dispatch(const Dispatcher<Key, Result> &d, const Key &key) {
  typename std::unordered_map<Key, std::function<Result()>>::const_iterator it =
      d.table.find(key);
  return it != d.table.end() ? just(it->second()) : nothing<Result>();
}

/**
 * @brief Looks up and invokes a handler or returns a typed error.
 *
 * @signature template <typename E, typename Key, typename Result> Either<E, Result> dispatch_either(const Dispatcher<Key, Result> &d, const Key &key, E error)
 *
 * User Story: As reducer and ECS code, I need strict dispatch misses to remain
 * explicit errors instead of falling through to hidden defaults.
 */
template <typename E, typename Key, typename Result>
Either<E, Result> dispatch_either(const Dispatcher<Key, Result> &d,
                                  const Key &key, E error) {
  Maybe<Result> result = dispatch(d, key);
  return result.hasValue ? make_right<E, Result>(result.value)
                         : make_left<E, Result>(error);
}

/**
 * @brief Reports whether a dispatcher has a handler for the given key.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Key, typename Result> bool has(const Dispatcher<Key, Result> &d, const Key &key)
 * 
 * User Story: As keyed dispatch flows, I need a presence check so callers can
 * branch before invoking optional handlers.
 */
template <typename Key, typename Result>
bool has(const Dispatcher<Key, Result> &d, const Key &key) {
  return d.table.find(key) != d.table.end();
}

/**
 * @brief Returns every key currently registered in the dispatcher.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Key, typename Result> std::vector<Key> keys(const Dispatcher<Key, Result> &d)
 * 
 * User Story: As keyed dispatch flows, I need access to registered keys so
 * tools and tests can inspect available handlers.
 */
template <typename Key, typename Result>
std::vector<Key> keys(const Dispatcher<Key, Result> &d) {
  return detail::dispatcherKeysRecursive<Key, Result>(d.table.begin(),
                                                      d.table.end(),
                                                      std::vector<Key>());
}

/**
 * @brief Strict keyed dispatcher for handlers that consume one argument.
 *
 * @signature template <typename Key, typename Arg, typename Result> struct ArgDispatcher
 *
 * User Story: As ECS and adapter code, I need keyed argument handlers without
 * fallback behavior so absent handlers return Maybe/Either at the boundary.
 */
template <typename Key, typename Arg, typename Result>
struct ArgDispatcher {
  std::unordered_map<Key, std::function<Result(const Arg &)>> table;
};

/**
 * @brief Request payload for strict argument dispatch.
 *
 * @signature template <typename Key, typename Arg, typename Result> struct ArgDispatcherDispatch
 *
 * User Story: As function-composition code, I need multi-input dispatch calls
 * carried through one payload while the dispatcher itself remains reusable.
 */
template <typename Key, typename Arg, typename Result>
struct ArgDispatcherDispatch {
  const ArgDispatcher<Key, Arg, Result> *dispatcher;
  const Key *key;
  const Arg *arg;
};

/**
 * @brief Creates an empty strict argument dispatcher.
 *
 * @signature template <typename Key, typename Arg, typename Result> ArgDispatcher<Key, Arg, Result> create_arg_dispatcher()
 *
 * User Story: As keyed dispatch setup code, I need an empty value that can be
 * extended through registration functions and lazy cached when needed.
 */
template <typename Key, typename Arg, typename Result>
ArgDispatcher<Key, Arg, Result> create_arg_dispatcher() {
  return ArgDispatcher<Key, Arg, Result>();
}

/**
 * @brief Registers a key-to-argument-handler entry.
 *
 * @signature template <typename Key, typename Arg, typename Result> ArgDispatcher<Key, Arg, Result> arg_dispatcher_register(ArgDispatcher<Key, Arg, Result> dispatcher, Key key, std::function<Result(const Arg &)> handler)
 *
 * User Story: As formatter and adapter code, I need registration to return the
 * next dispatcher value so tables compose through pipe/lazy helpers.
 */
template <typename Key, typename Arg, typename Result>
ArgDispatcher<Key, Arg, Result> arg_dispatcher_register(
    ArgDispatcher<Key, Arg, Result> dispatcher, Key key,
    std::function<Result(const Arg &)> handler) {
  dispatcher.table[std::move(key)] = std::move(handler);
  return dispatcher;
}

/**
 * @brief Looks up and invokes an argument handler when one exists.
 *
 * @signature template <typename Key, typename Arg, typename Result> Maybe<Result> arg_dispatcher_dispatch_maybe(const ArgDispatcherDispatch<Key, Arg, Result> &request)
 *
 * User Story: As ECS code, I need formatter and routing misses to be explicit
 * Maybe values instead of implicit fallback paths.
 */
template <typename Key, typename Arg, typename Result>
Maybe<Result> arg_dispatcher_dispatch_maybe(
    const ArgDispatcherDispatch<Key, Arg, Result> &request) {
  typename std::unordered_map<Key, std::function<Result(const Arg &)>>::
      const_iterator It = request.dispatcher->table.find(*request.key);
  return It != request.dispatcher->table.end() ? just(It->second(*request.arg))
                                               : nothing<Result>();
}

/**
 * @brief Looks up and invokes an argument handler or returns a typed error.
 *
 * @signature template <typename E, typename Key, typename Arg, typename Result> Either<E, Result> arg_dispatcher_dispatch_either(const ArgDispatcherDispatch<Key, Arg, Result> &request, E error)
 *
 * User Story: As reducer and ECS code, I need strict dispatch to return a typed
 * error when a table misses instead of silently choosing a default branch.
 */
template <typename E, typename Key, typename Arg, typename Result>
Either<E, Result> arg_dispatcher_dispatch_either(
    const ArgDispatcherDispatch<Key, Arg, Result> &request, E error) {
  Maybe<Result> result = arg_dispatcher_dispatch_maybe(request);
  return result.hasValue ? make_right<E, Result>(result.value)
                         : make_left<E, Result>(error);
}

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

namespace detail {
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
 * @brief Builds a match case from a predicate and a handler.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename R> MatchCase<T, R> when(std::function<bool(const T &)> pred, std::function<R(const T &)> handler)
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

template <typename T, typename R>
MatchCase<T, R> match_case(std::function<bool(const T &)> pred,
                           std::function<R(const T &)> handler) {
  return when<T, R>(std::move(pred), std::move(handler));
}

/**
 * @brief Returns a predicate that matches every input.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> std::function<bool(const T &)> wildcard()
 * 
 * User Story: As pattern-matching helpers, I need a wildcard predicate so
 * match lists can declare explicit default branches.
 */
template <typename T> std::function<bool(const T &)> wildcard() {
  return [](const T &) { return true; };
}

/**
 * @brief Returns a predicate that matches a specific expected value.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T> std::function<bool(const T &)> equals(T expected)
 * 
 * User Story: As pattern-matching helpers, I need equality predicates so case
 * lists can express direct value matches declaratively.
 */
template <typename T> std::function<bool(const T &)> equals(T expected) {
  return [expected](const T &value) { return value == expected; };
}

/**
 * @brief Evaluates match cases in order and returns the first successful result.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename T, typename R> Maybe<R> multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases)
 * 
 * User Story: As pattern-matching helpers, I need ordered case evaluation so
 * callers can express prioritized matching without manual branching.
 */
template <typename T, typename R>
Maybe<R> multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases) {
  return detail::multiMatchRecursive<T, R>(value, cases, 0);
}

template <typename T, typename R, typename FWildcard>
R multi_match(const T &value, const std::vector<MatchCase<T, R>> &cases,
              FWildcard wildcard) {
  return match(multi_match<T, R>(value, cases),
               [](const R &matched) { return matched; },
               [&value, &wildcard]() { return wildcard(value); });
}

template <typename T, typename R>
Maybe<R> multi_match_maybe(const T &value,
                           const std::vector<MatchCase<T, R>> &cases) {
  return multi_match<T, R>(value, cases);
}

template <typename Arg, typename Result> struct Bounce {
  bool bDone;
  Arg next;
  Result result;
};

/**
 * @brief Returns a Bounce object representing the next step in a trampoline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Arg, typename Result> Bounce<Arg, Result> call(Arg next)
 * 
 * User Story: As a functional programmer, I need a way to return the next step in a recursive function to avoid stack overflows.
 */
template <typename Arg, typename Result>
Bounce<Arg, Result> call(Arg next) {
  return Bounce<Arg, Result>{false, std::move(next), Result{}};
}

/**
 * @brief Returns a Bounce object representing the final result of a trampoline.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Arg, typename Result> Bounce<Arg, Result> done(Result result)
 * 
 * User Story: As a functional programmer, I need a way to return the final result of a recursive function to end the trampoline execution.
 */
template <typename Arg, typename Result>
Bounce<Arg, Result> done(Result result) {
  return Bounce<Arg, Result>{true, Arg{}, std::move(result)};
}

/**
 * @brief Executes a trampoline to evaluate a recursive function without blowing the stack.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 * 
 * @signature template <typename Arg, typename Result, typename Step> Result trampoline(Arg arg, Step step)
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

} // namespace func

#endif // UE_FP_HPP
