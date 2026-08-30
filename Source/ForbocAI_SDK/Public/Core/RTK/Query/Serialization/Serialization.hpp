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

/** User Story: As an RTK Query JSON consumer, I need each nullable string value lifted and type-refined before it enters domain state. @fn template <typename JsonValueT> func::Maybe<FString> deserializeStringValue(const TSharedPtr<JsonValueT> &JsonValue) */
template <typename JsonValueT>
func::Maybe<FString>
deserializeStringValue(const TSharedPtr<JsonValueT> &JsonValue) {
  return func::fmap(
      func::maybe_filter(
          func::fromNullable(JsonValue,
                             static_cast<bool>(JsonValue.Get())),
          [](const TSharedPtr<JsonValueT> &Value) {
            return Value->Type == EJson::String;
          }),
      [](const TSharedPtr<JsonValueT> &Value) { return Value->AsString(); });
}

/** User Story: As an RTK Query JSON consumer, I need each nullable object value decoded through one Maybe pipeline so malformed entries reject the complete array. @fn template <typename T, typename JsonValueT> func::Maybe<T> deserializeStructValue(const TSharedPtr<JsonValueT> &JsonValue) */
template <typename T, typename JsonValueT>
func::Maybe<T>
deserializeStructValue(const TSharedPtr<JsonValueT> &JsonValue) {
  return func::mbind(
      func::maybe_filter(
          func::fromNullable(JsonValue,
                             static_cast<bool>(JsonValue.Get())),
          [](const TSharedPtr<JsonValueT> &Value) {
            return Value->Type == EJson::Object;
          }),
      [](const TSharedPtr<JsonValueT> &Value) {
        const TSharedPtr<FJsonObject> JsonObject = Value->AsObject();
        return func::mbind(
            func::fromNullable(JsonObject,
                               static_cast<bool>(JsonObject.Get())),
            [](const TSharedPtr<FJsonObject> &Object) {
              T Item{};
              const bool bDecoded =
                  FJsonObjectConverter::JsonObjectToUStruct(
                      Object.ToSharedRef(), T::StaticStruct(), &Item,
                      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
                      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
              return func::fromNullable(MoveTemp(Item), bDecoded);
            });
      });
}

/** User Story: As an RTK Query JSON consumer, I need string arrays traversed atomically so one malformed member cannot leave a partially decoded result. @fn template <typename JsonValueT> func::Maybe<TArray<FString>> deserializeStringArray(const TArray<TSharedPtr<JsonValueT>> &JsonValues) */
template <typename JsonValueT>
func::Maybe<TArray<FString>>
deserializeStringArray(const TArray<TSharedPtr<JsonValueT>> &JsonValues) {
  return func::traverse_maybe_array(
      JsonValues, [](const TSharedPtr<JsonValueT> &JsonValue) {
        return deserializeStringValue(JsonValue);
      });
}

/** User Story: As an RTK Query JSON consumer, I need struct arrays traversed atomically so decoding either returns every domain value or no value. @fn template <typename T, typename JsonValueT> func::Maybe<TArray<T>> deserializeStructArray(const TArray<TSharedPtr<JsonValueT>> &JsonValues) */
template <typename T, typename JsonValueT>
func::Maybe<TArray<T>>
deserializeStructArray(const TArray<TSharedPtr<JsonValueT>> &JsonValues) {
  return func::traverse_maybe_array(
      JsonValues, [](const TSharedPtr<JsonValueT> &JsonValue) {
        return deserializeStructValue<T>(JsonValue);
      });
}

/** User Story: As an RTK Query deserializer boundary, I need a single effect adapter that commits a complete Maybe value to Unreal's required output parameter. @fn template <typename T> bool assignDeserializedValue(const func::Maybe<T> &Value, T &OutValue) */
template <typename T>
bool assignDeserializedValue(const func::Maybe<T> &Value, T &OutValue) {
  return func::match(
      Value,
      [&OutValue](const T &Decoded) {
        OutValue = Decoded;
        return true;
      },
      []() { return false; });
}
} // namespace detail
} // namespace rtk
