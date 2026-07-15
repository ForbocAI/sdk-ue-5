#pragma once

#include "Core/FP/Indexed/Indexed.hpp"
#include "Core/FP/Monad/Monad.hpp"

namespace func {
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

} // namespace func
