#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/JsonInterop/Parsing/ParsingAdapters.h"

namespace JsonInterop {
namespace detail {

/** User Story: As a JSON field consumer, I need nullable objects lifted once so field operations compose through Maybe instead of repeating pointer branches. @fn inline func::Maybe<TSharedPtr<FJsonObject>> PresentObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<TSharedPtr<FJsonObject>>
PresentObject(const TSharedPtr<FJsonObject> &Object) {
  return func::fromNullable(Object, static_cast<bool>(Object.Get()));
}

/** User Story: As a JSON field consumer, I need nullable field lookup represented as Maybe so missing and present values have one composable shape. @fn inline func::Maybe<TSharedPtr<FJsonValue>> FieldValue(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline func::Maybe<TSharedPtr<FJsonValue>>
FieldValue(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) {
  return func::mbind(
      PresentObject(Object),
      [&FieldName](const TSharedPtr<FJsonObject> &Present) {
        const TSharedPtr<FJsonValue> Value = Present->TryGetField(FieldName);
        return func::fromNullable(Value, static_cast<bool>(Value.Get()));
      });
}

/** User Story: As a JSON field consumer, I need explicit null values filtered from present values before domain decoding. @fn inline func::Maybe<TSharedPtr<FJsonValue>> NonNullFieldValue(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline func::Maybe<TSharedPtr<FJsonValue>>
NonNullFieldValue(const TSharedPtr<FJsonObject> &Object,
                  const FString &FieldName) {
  return func::maybe_filter(
      FieldValue(Object, FieldName),
      [](const TSharedPtr<FJsonValue> &Value) {
        return Value->Type != EJson::Null;
      });
}

/** User Story: As a JSON field consumer, I need string values projected directly and other JSON values serialized through the canonical interop boundary. @fn inline FString StringValue(const TSharedPtr<FJsonValue> &Value) */
inline FString StringValue(const TSharedPtr<FJsonValue> &Value) {
  return func::multi_match<EJson, FString>(
      Value->Type,
      {func::when<EJson, FString>(
          func::equals<EJson>(EJson::String),
          [&Value](const EJson &) { return Value->AsString(); })},
      [&Value](const EJson &) { return StringifyValue(Value); });
}

} // namespace detail

/** User Story: As a core json interop fields consumer, I need to invoke has non null field through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline bool HasNonNullField(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline bool HasNonNullField(const TSharedPtr<FJsonObject> &Object,
                            const FString &FieldName) {
  return func::match(
      detail::NonNullFieldValue(Object, FieldName),
      [](const TSharedPtr<FJsonValue> &) { return true; },
      []() { return false; });
}

/** User Story: As a JSON boundary consumer, I need nullable optional fields checked without coercion so malformed wire values cannot enter domain state. @fn inline bool HasOptionalFieldType(const TSharedPtr<FJsonObject> &Object, const FString &FieldName, EJson ExpectedType) */
inline bool HasOptionalFieldType(const TSharedPtr<FJsonObject> &Object,
                                 const FString &FieldName,
                                 EJson ExpectedType) {
  return func::match(
      detail::PresentObject(Object),
      [&Object, &FieldName,
       ExpectedType](const TSharedPtr<FJsonObject> &) {
        return func::match(
            detail::FieldValue(Object, FieldName),
            [ExpectedType](const TSharedPtr<FJsonValue> &Value) {
              return Value->Type == EJson::Null ||
                     Value->Type == ExpectedType;
            },
            []() { return true; });
      },
      []() { return false; });
}

/** User Story: As a core json interop fields consumer, I need to invoke optional string from field through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline FString OptionalStringFromField( const TSharedPtr<FJsonObject> &Object, const FString &FieldName, const FString &DefaultValue = TEXT("")) */
inline FString OptionalStringFromField(
    const TSharedPtr<FJsonObject> &Object, const FString &FieldName,
    const FString &DefaultValue = TEXT("")) {
  return func::match(
      detail::NonNullFieldValue(Object, FieldName),
      [](const TSharedPtr<FJsonValue> &Value) {
        return detail::StringValue(Value);
      },
      [&DefaultValue]() { return DefaultValue; });
}

/** User Story: As a typed JSON consumer, I need string fields refined into Maybe so malformed and missing values remain outside domain state. @fn inline func::Maybe<FString> StringFieldValue(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline func::Maybe<FString>
StringFieldValue(const TSharedPtr<FJsonObject> &Object,
                 const FString &FieldName) {
  return func::fmap(
      func::maybe_filter(
          detail::NonNullFieldValue(Object, FieldName),
          [](const TSharedPtr<FJsonValue> &Value) {
            return Value->Type == EJson::String;
          }),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsString(); });
}

/** User Story: As a typed JSON consumer, I need array fields refined into Maybe so nullable engine pointers never escape the interop boundary. @fn inline func::Maybe<TArray<TSharedPtr<FJsonValue>>> ArrayFieldValues(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline func::Maybe<TArray<TSharedPtr<FJsonValue>>>
ArrayFieldValues(const TSharedPtr<FJsonObject> &Object,
                 const FString &FieldName) {
  return func::fmap(
      func::maybe_filter(
          detail::NonNullFieldValue(Object, FieldName),
          [](const TSharedPtr<FJsonValue> &Value) {
            return Value->Type == EJson::Array;
          }),
      [](const TSharedPtr<FJsonValue> &Value) { return Value->AsArray(); });
}

/** User Story: As a core json interop fields consumer, I need to invoke parse json object or empty through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline TSharedPtr<FJsonObject> ParseJsonObjectOrEmpty(const FString &Json) */
inline TSharedPtr<FJsonObject> ParseJsonObjectOrEmpty(const FString &Json) {
  TSharedPtr<FJsonObject> Object;
  ParseJsonObject(Json, Object);
  return Object;
}

/**
 * User Story: As a core json interop fields consumer, I need to invoke json string from field through a stable signature so the core json interop fields workflow remains explicit and composable.
 * @fn inline FString JsonStringFromField( const TSharedPtr<FJsonObject> &Object, const FString &FieldName, const FString &DefaultValue = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF54CAD9838EB))
 */
inline FString JsonStringFromField(
    const TSharedPtr<FJsonObject> &Object, const FString &FieldName,
    const FString &DefaultValue = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF54CAD9838EB)) {
  return OptionalStringFromField(Object, FieldName, DefaultValue);
}

/** User Story: As a core json interop fields consumer, I need to invoke set field from json string through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline void SetFieldFromJsonString(const TSharedRef<FJsonObject> &Object, const FString &FieldName, const FString &JsonString, bool bOmitWhenEmpty = true) */
inline void SetFieldFromJsonString(const TSharedRef<FJsonObject> &Object,
                                   const FString &FieldName,
                                   const FString &JsonString,
                                   bool bOmitWhenEmpty = true) {
  JsonString.IsEmpty()
      ? (bOmitWhenEmpty
             ? void()
             : (Object->SetObjectField(FieldName, MakeShared<FJsonObject>()),
                void()))
      : [&]() {
          TSharedPtr<FJsonObject> JsonObject;
          ParseJsonObject(JsonString, JsonObject)
              ? (Object->SetObjectField(FieldName, JsonObject), void())
              : [&]() {
                  TArray<TSharedPtr<FJsonValue>> JsonArray;
                  ParseJsonArray(JsonString, JsonArray)
                      ? (Object->SetArrayField(FieldName, JsonArray), void())
                      : ((!bOmitWhenEmpty || !JsonString.IsEmpty())
                             ? (Object->SetStringField(FieldName, JsonString),
                                void())
                             : void());
                }();
        }();
}

} // namespace JsonInterop
