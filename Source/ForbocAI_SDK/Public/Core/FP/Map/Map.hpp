#pragma once

#include "Core/FP/Array/Array.hpp"

namespace func {
/** User Story: As a core fp map consumer, I need to invoke map keys through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> TArray<Key> map_keys(const TMap<Key, Value> &values) */
template <typename Key, typename Value>
TArray<Key> map_keys(const TMap<Key, Value> &values) {
  TArray<Key> keys;
  values.GetKeys(keys);
  return keys;
}

/** User Story: As a core fp map consumer, I need to invoke append map keys through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> TArray<Key> append_map_keys(TArray<Key> values, const TMap<Key, Value> &map) */
template <typename Key, typename Value>
TArray<Key> append_map_keys(TArray<Key> values,
                            const TMap<Key, Value> &map) {
  return append_values<Key>(values, map_keys<Key, Value>(map));
}

/** User Story: As a core fp map consumer, I need to invoke find map value ptr through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> Maybe<const Value *> find_map_value_ptr(const TMap<Key, Value> &values, const Key &key) */
template <typename Key, typename Value>
Maybe<const Value *> find_map_value_ptr(const TMap<Key, Value> &values,
                                        const Key &key) {
  const Value *found = values.Find(key);
  return found ? just<const Value *>(found) : nothing<const Value *>();
}

/** User Story: As a core fp map consumer, I need to invoke find map value through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> Maybe<Value> find_map_value(const TMap<Key, Value> &values, const Key &key) */
template <typename Key, typename Value>
Maybe<Value> find_map_value(const TMap<Key, Value> &values, const Key &key) {
  return match(
      find_map_value_ptr<Key, Value>(values, key),
      [](const Value *found) { return just<Value>(*found); },
      []() { return nothing<Value>(); });
}

/** User Story: As a core fp map consumer, I need to invoke map value or through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> Value map_value_or(const TMap<Key, Value> &values, const Key &key, const Value &defaultValue) */
template <typename Key, typename Value>
Value map_value_or(const TMap<Key, Value> &values, const Key &key,
                   const Value &defaultValue) {
  return or_else(find_map_value<Key, Value>(values, key), defaultValue);
}

/** User Story: As a core fp map consumer, I need to invoke update map value when present through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value, typename Transform> TMap<Key, Value> update_map_value_when_present(TMap<Key, Value> values, const Key &key, Transform transform) */
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

/** User Story: As a core fp map consumer, I need to invoke upsert map value through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value, typename Transform> TMap<Key, Value> upsert_map_value(TMap<Key, Value> values, const Key &key, const Value &defaultValue, Transform transform) */
template <typename Key, typename Value, typename Transform>
TMap<Key, Value> upsert_map_value(TMap<Key, Value> values, const Key &key,
                                  const Value &defaultValue,
                                  Transform transform) {
  values.Add(key,
             transform(map_value_or<Key, Value>(values, key, defaultValue)));
  return values;
}

/** User Story: As a core fp map consumer, I need to invoke map has key through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value> std::function<bool(const Key &)> map_has_key(const TMap<Key, Value> &values) */
template <typename Key, typename Value>
std::function<bool(const Key &)> map_has_key(const TMap<Key, Value> &values) {
  return [&values](const Key &key) { return values.Contains(key); };
}

/** User Story: As a core fp map consumer, I need to invoke map array contains through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Item> bool map_array_contains(const TMap<Key, TArray<Item>> &values, const Key &key, const Item &item) */
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

/** User Story: As a core fp map consumer, I need to invoke map values equal through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value, typename Equals> bool map_values_equal(const TMap<Key, Value> &left, const TMap<Key, Value> &right, Equals equals) */
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

/** User Story: As a core fp map consumer, I need to invoke require map key through a stable signature so the core fp map workflow remains explicit and composable. @fn template <typename Key, typename Value, typename ErrorMessage> auto require_map_key(const TMap<Key, Value> &values, ErrorMessage error_message) -> std::function<Either<decltype(error_message( std::declval<const Key &>())), bool>(const Key &)> */
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

} // namespace func
