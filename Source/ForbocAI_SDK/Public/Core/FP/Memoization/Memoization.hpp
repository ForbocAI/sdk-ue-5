#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
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

/** User Story: As a core fp memoization consumer, I need to invoke call memoized last through a stable signature so the core fp memoization workflow remains explicit and composable. @fn template <typename Result, typename... Args> const Result &callMemoizedLast(const MemoizedLast<Result(Args...)> &memoized, Args... args) */
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

  /** User Story: As a core fp memoization consumer, I need to invoke the callable value through a stable signature so the core fp memoization workflow remains explicit and composable. @fn const Result &operator()(Args... args) const */
  const Result &operator()(Args... args) const {
    return detail::callMemoizedLast(*this, std::forward<Args>(args)...);
  }
};

namespace detail {
template <typename Result, typename... Args>
struct MemoizedLastFactory<Result(Args...)> {
  typedef MemoizedLast<Result(Args...)> MemoizedType;

  /** User Story: As a core fp memoization consumer, I need to invoke default comparator through a stable signature so the core fp memoization workflow remains explicit and composable. @fn static typename MemoizedType::Comparator defaultComparator() */
  static typename MemoizedType::Comparator defaultComparator() {
    return typename MemoizedType::Comparator(
        [](const typename MemoizedType::ArgsTuple &lhs,
           const typename MemoizedType::ArgsTuple &rhs) { return lhs == rhs; });
  }

  /** User Story: As a core fp memoization consumer, I need to invoke create through a stable signature so the core fp memoization workflow remains explicit and composable. @fn static MemoizedType create(std::function<Result(Args...)> function, typename MemoizedType::Comparator comparator = defaultComparator()) */
  static MemoizedType
  create(std::function<Result(Args...)> function,
         typename MemoizedType::Comparator comparator = defaultComparator()) {
    MemoizedType memoized;
    memoized.func = std::move(function);
    memoized.equals = std::move(comparator);
    return memoized;
  }
};

/** User Story: As a core fp memoization consumer, I need to invoke store memoized result through a stable signature so the core fp memoization workflow remains explicit and composable. @fn template <typename Result> const Result &storeMemoizedResult(std::shared_ptr<Result> &target, Result computed) */
template <typename Result>
const Result &storeMemoizedResult(std::shared_ptr<Result> &target,
                                  Result computed) {
  return target ? (*target = std::move(computed), *target)
                : *(target = std::make_shared<Result>(std::move(computed)));
}

/** User Story: As a core fp memoization consumer, I need to invoke recompute memoized last through a stable signature so the core fp memoization workflow remains explicit and composable. @fn template <typename Result, typename... Args> const Result & recomputeMemoizedLast(const MemoizedLast<Result(Args...)> &memoized, typename MemoizedLast<Result(Args...)>::ArgsTuple current) */
template <typename Result, typename... Args>
const Result &
recomputeMemoizedLast(const MemoizedLast<Result(Args...)> &memoized,
                      typename MemoizedLast<Result(Args...)>::ArgsTuple current) {
  Result computed = func::apply(memoized.func, current);
  memoized.lastArgs = current;
  memoized.hasCached = true;
  return storeMemoizedResult(memoized.lastResult, std::move(computed));
}

/** User Story: As a core fp memoization consumer, I need to invoke call memoized last through a stable signature so the core fp memoization workflow remains explicit and composable. @fn template <typename Result, typename... Args> const Result &callMemoizedLast(const MemoizedLast<Result(Args...)> &memoized, Args... args) */
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
 * @fn template <typename Signature> MemoizedLast<Signature> memoizeLast(std::function<Signature> function)
 * @brief Memoizes the last invocation of a std::function with default comparison.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
 *
 * User Story: As derived-data helpers, I need last-call memoization so cached
 * computations can be reused when inputs repeat.
 */
template <typename Signature>
MemoizedLast<Signature> memoizeLast(std::function<Signature> function) {
  return detail::MemoizedLastFactory<Signature>::create(std::move(function));
}

/**
 * @fn template <typename Signature, typename F> MemoizedLast<Signature> memoizeLast(F f)
 * @brief Memoizes the last invocation of a generic callable with default comparison.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
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
 * @fn template <typename Signature> MemoizedLast<Signature> memoizeLast(std::function<Signature> function, typename MemoizedLast<Signature>::Comparator comparator)
 * @brief Memoizes the last invocation using a custom argument comparator.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
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
 * @fn template <typename Signature, typename F> MemoizedLast<Signature> memoizeLast(F f, typename MemoizedLast<Signature>::Comparator comparator)
 * @brief Memoizes a generic callable using a custom argument comparator.
 *
 * @details This component is part of the strict C++11 functional core library, providing functional programming primitives without relying on newer language features.
 *
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

} // namespace func
