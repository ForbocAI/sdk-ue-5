#pragma once

#include "Core/FP/Indexed/Indexed.hpp"
#include "Core/FP/Monad/Monad.hpp"

namespace func {
/**
 * @fn template <typename Item, typename Acc, typename Step> Acc fold_array(const TArray<Item> &values, Acc seed, Step step)
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

/** User Story: As a core fp array consumer, I need to invoke for each array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item, typename Effect> void for_each_array(const TArray<Item> &values, Effect effect) */
template <typename Item, typename Effect>
void for_each_array(const TArray<Item> &values, Effect effect) {
  for_each_indexed<TArray<Item>, Effect>(
      values, static_cast<size_t>(values.Num()), effect);
}

/** User Story: As a core fp array consumer, I need to invoke all array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item, typename Predicate> bool all_array(const TArray<Item> &values, Predicate predicate) */
template <typename Item, typename Predicate>
bool all_array(const TArray<Item> &values, Predicate predicate) {
  return all_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

/** User Story: As a core fp array consumer, I need to invoke any array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item, typename Predicate> bool any_array(const TArray<Item> &values, Predicate predicate) */
template <typename Item, typename Predicate>
bool any_array(const TArray<Item> &values, Predicate predicate) {
  return any_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

/** User Story: As a core fp array consumer, I need to invoke contains value through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> bool contains_value(const TArray<Item> &values, const Item &expected) */
template <typename Item>
bool contains_value(const TArray<Item> &values, const Item &expected) {
  return any_array<Item>(
      values, [&expected](const Item &value) { return value == expected; });
}

/** User Story: As a core fp array consumer, I need to invoke find array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item, typename Predicate> Maybe<Item> find_array(const TArray<Item> &values, Predicate predicate) */
template <typename Item, typename Predicate>
Maybe<Item> find_array(const TArray<Item> &values, Predicate predicate) {
  return find_indexed<TArray<Item>, Predicate>(
      values, static_cast<size_t>(values.Num()), predicate);
}

/** User Story: As a core fp array consumer, I need to invoke append value through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> TArray<Item> append_value(TArray<Item> values, const Item &value) */
template <typename Item>
TArray<Item> append_value(TArray<Item> values, const Item &value) {
  values.Add(value);
  return values;
}

/** User Story: As a core fp array consumer, I need to invoke append unique value through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> TArray<Item> append_unique_value(TArray<Item> values, const Item &value) */
template <typename Item>
TArray<Item> append_unique_value(TArray<Item> values, const Item &value) {
  values.AddUnique(value);
  return values;
}

/** User Story: As a core fp array consumer, I need to invoke append values through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> TArray<Item> append_values(TArray<Item> values, const TArray<Item> &additional_values) */
template <typename Item>
TArray<Item> append_values(TArray<Item> values,
                           const TArray<Item> &additional_values) {
  values.Append(additional_values);
  return values;
}

/** User Story: As a core fp array consumer, I need to invoke filter array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item, typename Predicate> TArray<Item> filter_array(const TArray<Item> &values, Predicate predicate) */
template <typename Item, typename Predicate>
TArray<Item> filter_array(const TArray<Item> &values, Predicate predicate) {
  return fold_array<Item, TArray<Item>>(
      values, TArray<Item>(),
      [predicate](const TArray<Item> &acc, const Item &value) {
        return predicate(value) ? append_value<Item>(acc, value) : acc;
      });
}

/**
 * User Story: As an FP collection consumer, I need bounded array ranges copied
 * without feature-local index loops so slicing remains portable and reusable.
 * @fn template <typename Item> TArray<Item> slice_array(const TArray<Item> &values, int32 start, int32 count)
 */
template <typename Item>
TArray<Item> slice_array(const TArray<Item> &values, int32 start, int32 count) {
  const int32 safe_start = FMath::Clamp(start, int32{}, values.Num());
  const int32 safe_count =
      FMath::Clamp(count, int32{}, values.Num() - safe_start);
  TArray<Item> result;
  return safe_count == int32{}
             ? result
             : (result.Append(values.GetData() + safe_start, safe_count),
                result);
}

/**
 * User Story: As an FP collection consumer, I need a pure prefix operation so
 * feature code can express bounded result sets without owning traversal.
 * @fn template <typename Item> TArray<Item> take_array(const TArray<Item> &values, int32 count)
 */
template <typename Item>
TArray<Item> take_array(const TArray<Item> &values, int32 count) {
  return slice_array(values, int32{}, count);
}

/**
 * User Story: As an FP collection consumer, I need a pure suffix operation so
 * feature code can advance through arrays without owning traversal.
 * @fn template <typename Item> TArray<Item> drop_array(const TArray<Item> &values, int32 count)
 */
template <typename Item>
TArray<Item> drop_array(const TArray<Item> &values, int32 count) {
  return slice_array(values, count, values.Num() - count);
}

/** User Story: As a core fp array consumer, I need to invoke map array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Source, typename Map> auto map_array(const TArray<Source> &values, Map map) -> TArray<decltype(map(std::declval<const Source &>()))> */
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

/** User Story: As a core fp array consumer, I need to invoke map array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Source, typename Output, typename Map> TArray<Output> map_array(const TArray<Source> &values, Map map) */
template <typename Source, typename Output, typename Map>
TArray<Output> map_array(const TArray<Source> &values, Map map) {
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [map](const TArray<Output> &acc, const Source &value) {
        return append_value<Output>(acc, map(value));
      });
}

/** User Story: As a core fp array consumer, I need to invoke filter map array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Source, typename Keep, typename Map> auto filter_map_array(const TArray<Source> &values, Keep keep, Map map) -> TArray<decltype(map(std::declval<const Source &>()))> */
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

/** User Story: As a core fp array consumer, I need to invoke filter map array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Source, typename Output, typename Keep, typename Map> TArray<Output> filter_map_array(const TArray<Source> &values, Keep keep, Map map) */
template <typename Source, typename Output, typename Keep, typename Map>
TArray<Output> filter_map_array(const TArray<Source> &values, Keep keep,
                                Map map) {
  return fold_array<Source, TArray<Output>>(
      values, TArray<Output>(),
      [keep, map](const TArray<Output> &acc, const Source &value) {
        return keep(value) ? append_value<Output>(acc, map(value)) : acc;
      });
}

/** User Story: As a core fp array consumer, I need to invoke fmap through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename T, typename Func> auto fmap(const TArray<T> &values, Func f) -> TArray<decltype(f(std::declval<const T &>()))> */
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

/** User Story: As a core fp array consumer, I need to invoke traverse maybe array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Source, typename Output, typename Map> Maybe<TArray<Output>> traverse_maybe_array(const TArray<Source> &values, Map map) */
template <typename Source, typename Output, typename Map>
Maybe<TArray<Output>> traverse_maybe_array(const TArray<Source> &values,
                                           Map map) {
  return traverse_maybe_array(values, map);
}

/** User Story: As a core fp array consumer, I need to invoke sequence maybe array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename T> Maybe<TArray<T>> sequence_maybe_array(const TArray<Maybe<T>> &values) */
template <typename T>
Maybe<TArray<T>> sequence_maybe_array(const TArray<Maybe<T>> &values) {
  return traverse_maybe_array(
      values, [](const Maybe<T> &value) { return value; });
}

/** User Story: As a core fp array consumer, I need to invoke fold either array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename E, typename Item, typename Acc, typename Step> Either<E, Acc> fold_either_array(const TArray<Item> &values, Acc seed, Step step) */
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

/** User Story: As a core fp array consumer, I need to invoke concat arrays through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> TArray<Item> concat_arrays(const TArray<TArray<Item>> &arrays) */
template <typename Item>
TArray<Item> concat_arrays(const TArray<TArray<Item>> &arrays) {
  return fold_array<TArray<Item>, TArray<Item>>(
      arrays, TArray<Item>(),
      [](const TArray<Item> &acc, const TArray<Item> &values) {
        return append_values<Item>(acc, values);
      });
}

/** User Story: As a core fp array consumer, I need to invoke unique array through a stable signature so the core fp array workflow remains explicit and composable. @fn template <typename Item> TArray<Item> unique_array(const TArray<Item> &values) */
template <typename Item>
TArray<Item> unique_array(const TArray<Item> &values) {
  return fold_array<Item, TArray<Item>>(
      values, TArray<Item>(),
      [](const TArray<Item> &acc, const Item &value) {
        return append_unique_value<Item>(acc, value);
      });
}

} // namespace func
