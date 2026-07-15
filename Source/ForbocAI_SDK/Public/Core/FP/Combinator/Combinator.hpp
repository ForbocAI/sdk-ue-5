#pragma once

#include "Core/FP/Composition/Composition.hpp"

namespace func {
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

} // namespace func
