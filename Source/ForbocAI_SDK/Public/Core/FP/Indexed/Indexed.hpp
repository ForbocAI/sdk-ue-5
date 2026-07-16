#pragma once

#include "Core/FP/Maybe/Maybe.hpp"
#include "Core/FP/Combinator/Combinator.hpp"

namespace func {
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
/** User Story: As a core fp indexed consumer, I need to invoke fold indexed recursive through a stable signature so the core fp indexed workflow remains explicit and composable. @fn template <typename IndexedCollection, typename Acc, typename Step> Acc foldIndexedRecursive(const IndexedCollection &values, size_t count, size_t index, Acc acc, Step step) */
template <typename IndexedCollection, typename Acc, typename Step>
Acc foldIndexedRecursive(const IndexedCollection &values, size_t count,
                         size_t index, Acc acc, Step step) {
  return index >= count
             ? acc
             : foldIndexedRecursive<IndexedCollection, Acc, Step>(
                   values, count, index + 1, step(acc, values[index]), step);
}

/** User Story: As a core fp indexed consumer, I need to invoke find indexed recursive through a stable signature so the core fp indexed workflow remains explicit and composable. @fn template <typename IndexedCollection, typename Predicate, typename Value> Maybe<Value> findIndexedRecursive(const IndexedCollection &values, size_t count, size_t index, Predicate predicate) */
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

/** User Story: As a core fp indexed consumer, I need to invoke map grid recursive through a stable signature so the core fp indexed workflow remains explicit and composable. @fn template <typename Output, typename Map> std::vector<Output> mapGridRecursive(size_t rows, size_t columns, size_t index, Map map, std::vector<Output> result) */
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
 * @fn template <typename IndexedCollection, typename Acc, typename Step> Acc fold_indexed(const IndexedCollection &values, size_t count, Acc seed, Step step)
 * @brief Folds any indexed collection through a pure accumulator step.
 *
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
 * @fn template <typename IndexedCollection, typename Effect> void for_each_indexed(const IndexedCollection &values, size_t count, Effect effect)
 * @brief Runs one effect for each indexed collection value.
 *
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
 * @fn template <typename IndexedCollection, typename Predicate> auto find_indexed(const IndexedCollection &values, size_t count, Predicate predicate) -> Maybe<typename std::decay<decltype(std::declval< const IndexedCollection &>()[0])>:: type>
 * @brief Finds the first indexed collection value that satisfies a predicate.
 *
 *
 * User Story: As lookup code, I need indexed collection searches to return
 * Maybe values without hidden sentinels.
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
 * @fn template <typename IndexedCollection, typename Predicate> bool any_indexed(const IndexedCollection &values, size_t count, Predicate predicate)
 * @brief Returns true when any indexed collection value satisfies a predicate.
 *
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
 * @fn template <typename IndexedCollection, typename Predicate> bool all_indexed(const IndexedCollection &values, size_t count, Predicate predicate)
 * @brief Returns true when every indexed collection value satisfies a predicate.
 *
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
 * @fn template <typename Output, typename Map> std::vector<Output> map_grid(size_t rows, size_t columns, Map map)
 * @brief Maps a rectangular grid into a vector in row-major order.
 *
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

} // namespace func
