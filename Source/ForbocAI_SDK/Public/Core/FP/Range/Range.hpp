#pragma once

#include "Core/FP/Array/Array.hpp"

namespace func {
/** User Story: As a core fp range consumer, I need to invoke fold index range through a stable signature so the core fp range workflow remains explicit and composable. @fn template <typename Acc, typename Step> Acc fold_index_range(int32 count, Acc seed, Step step, int32 index = 0) */
template <typename Acc, typename Step>
Acc fold_index_range(int32 count, Acc seed, Step step, int32 index = 0) {
  return index >= count
             ? seed
             : fold_index_range<Acc, Step>(count, step(seed, index), step,
                                           index + 1);
}

/** User Story: As a core fp range consumer, I need to invoke index range through a stable signature so the core fp range workflow remains explicit and composable. @fn inline TArray<int32> index_range(int32 count) */
inline TArray<int32> index_range(int32 count) {
  return fold_index_range<TArray<int32>>(
      count, TArray<int32>(),
      [](const TArray<int32> &acc, int32 index) {
        return append_value<int32>(acc, index);
      });
}

/** User Story: As a core fp range consumer, I need to invoke map index range through a stable signature so the core fp range workflow remains explicit and composable. @fn template <typename Map> auto map_index_range(int32 count, Map map) -> TArray<decltype(map(std::declval<int32>()))> */
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

/** User Story: As a core fp range consumer, I need to invoke traverse maybe array with index through a stable signature so the core fp range workflow remains explicit and composable. @fn template <typename Source, typename Output, typename Map> Maybe<TArray<Output>> traverse_maybe_array_with_index(const TArray<Source> &values, Map map) */
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

/** User Story: As a core fp range consumer, I need to invoke fold grid range through a stable signature so the core fp range workflow remains explicit and composable. @fn template <typename Acc, typename Step> Acc fold_grid_range(size_t rows, size_t columns, Acc seed, Step step) */
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

/** User Story: As a core fp range consumer, I need to invoke map grid array through a stable signature so the core fp range workflow remains explicit and composable. @fn template <typename Output, typename Map> TArray<Output> map_grid_array(size_t rows, size_t columns, Map map) */
template <typename Output, typename Map>
TArray<Output> map_grid_array(size_t rows, size_t columns, Map map) {
  return fold_grid_range<TArray<Output>>(
      rows, columns, TArray<Output>(),
      [map](const TArray<Output> &acc, const GridIndex &index) {
        return append_value<Output>(acc, map(index));
      });
}

} // namespace func
