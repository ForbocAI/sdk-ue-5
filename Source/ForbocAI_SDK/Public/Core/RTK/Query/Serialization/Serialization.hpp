#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Query/Prelude.hpp"
#include "Core/RTK/Query/Types/Types.hpp"

namespace rtk {
namespace detail {
template <typename T, typename = void> struct HasEqualOperator : std::false_type {};

template <typename T>
struct HasEqualOperator<
    T, typename std::enable_if<
           std::is_convertible<decltype(std::declval<const T &>() ==
                                        std::declval<const T &>()),
                               bool>::value>::type> : std::true_type {};

/** User Story: As a rtk query serialization consumer, I need to invoke copy with structural sharing impl through a stable signature so the rtk query serialization workflow remains explicit and composable. @fn template <typename T> T copyWithStructuralSharingImpl(const T &OldValue, const T &NewValue, std::true_type) */
template <typename T>
T copyWithStructuralSharingImpl(const T &OldValue, const T &NewValue,
                                std::true_type) {
  return OldValue == NewValue ? OldValue : NewValue;
}

/** User Story: As a rtk query serialization consumer, I need to invoke copy with structural sharing impl through a stable signature so the rtk query serialization workflow remains explicit and composable. @fn template <typename T> T copyWithStructuralSharingImpl(const T &, const T &NewValue, std::false_type) */
template <typename T>
T copyWithStructuralSharingImpl(const T &, const T &NewValue,
                                std::false_type) {
  return NewValue;
}

template <typename T, typename Enable = void> struct JsonDeserializer;

/** User Story: As a rtk query serialization consumer, I need to invoke deserialize string array recursive through a stable signature so the rtk query serialization workflow remains explicit and composable. @fn template <typename JsonValueT> bool deserializeStringArrayRecursive( const TArray<TSharedPtr<JsonValueT>> &JsonValues, int32 Index, TArray<FString> &OutValue) */
template <typename JsonValueT>
bool deserializeStringArrayRecursive(
    const TArray<TSharedPtr<JsonValueT>> &JsonValues, int32 Index,
    TArray<FString> &OutValue) {
  return Index == JsonValues.Num()
             ? true
             : !JsonValues[Index].IsValid()
                   ? false
                   : (OutValue.Add(JsonValues[Index]->AsString()),
                      deserializeStringArrayRecursive(JsonValues, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                                      OutValue));
}

/** User Story: As a rtk query serialization consumer, I need to invoke deserialize struct array recursive through a stable signature so the rtk query serialization workflow remains explicit and composable. @fn template <typename T, typename JsonValueT> bool deserializeStructArrayRecursive( const TArray<TSharedPtr<JsonValueT>> &JsonValues, int32 Index, TArray<T> &OutValue) */
template <typename T, typename JsonValueT>
bool deserializeStructArrayRecursive(
    const TArray<TSharedPtr<JsonValueT>> &JsonValues, int32 Index,
    TArray<T> &OutValue) {
  const TSharedPtr<FJsonObject> JsonObject =
      Index == JsonValues.Num()
          ? TSharedPtr<FJsonObject>()
          : (JsonValues[Index].IsValid() ? JsonValues[Index]->AsObject()
                                         : TSharedPtr<FJsonObject>());
  T Item;
  return Index == JsonValues.Num()
             ? true
             : !JsonValues[Index].IsValid()
                   ? false
                   : !JsonObject.IsValid()
                         ? false
                         : !FJsonObjectConverter::JsonObjectToUStruct(
                               JsonObject.ToSharedRef(), T::StaticStruct(),
                               &Item, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA)
                               ? false
                               : (OutValue.Add(Item),
                                  deserializeStructArrayRecursive<T>(
                                      JsonValues, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, OutValue));
}
} // namespace detail
} // namespace rtk
