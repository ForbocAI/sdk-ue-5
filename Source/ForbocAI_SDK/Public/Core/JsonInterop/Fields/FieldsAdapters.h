#pragma once

#include "Core/JsonInterop/Parsing/ParsingAdapters.h"

namespace JsonInterop {

/** User Story: As a core json interop fields consumer, I need to invoke has non null field through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline bool HasNonNullField(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline bool HasNonNullField(const TSharedPtr<FJsonObject> &Object,
                            const FString &FieldName) {
  return !Object.IsValid() || !Object->HasField(FieldName)
             ? false
             : [&]() {
                 const TSharedPtr<FJsonValue> Value =
                     Object->TryGetField(FieldName);
                 return Value.IsValid() && Value->Type != EJson::Null;
               }();
}

/** User Story: As a JSON boundary consumer, I need nullable optional fields checked without coercion so malformed wire values cannot enter domain state. @fn inline bool HasOptionalFieldType(const TSharedPtr<FJsonObject> &Object, const FString &FieldName, EJson ExpectedType) */
inline bool HasOptionalFieldType(const TSharedPtr<FJsonObject> &Object,
                                 const FString &FieldName,
                                 EJson ExpectedType) {
  return !Object.IsValid()
             ? false
             : [&]() {
                 const TSharedPtr<FJsonValue> Value =
                     Object->TryGetField(FieldName);
                 return !Value.IsValid() || Value->Type == EJson::Null ||
                        Value->Type == ExpectedType;
               }();
}

/** User Story: As a core json interop fields consumer, I need to invoke optional string from field through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline FString OptionalStringFromField( const TSharedPtr<FJsonObject> &Object, const FString &FieldName, const FString &DefaultValue = TEXT("")) */
inline FString OptionalStringFromField(
    const TSharedPtr<FJsonObject> &Object, const FString &FieldName,
    const FString &DefaultValue = TEXT("")) {
  return !HasNonNullField(Object, FieldName)
             ? DefaultValue
             : [&]() {
                 const TSharedPtr<FJsonValue> Value =
                     Object->TryGetField(FieldName);
                 return !Value.IsValid()
                            ? DefaultValue
                            : Value->Type == EJson::String
                                  ? Value->AsString()
                                  : StringifyValue(Value);
               }();
}

/** User Story: As a core json interop fields consumer, I need to invoke parse json object or empty through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline TSharedPtr<FJsonObject> ParseJsonObjectOrEmpty(const FString &Json) */
inline TSharedPtr<FJsonObject> ParseJsonObjectOrEmpty(const FString &Json) {
  TSharedPtr<FJsonObject> Object;
  ParseJsonObject(Json, Object);
  return Object.IsValid() ? Object : MakeShared<FJsonObject>();
}

/** User Story: As a core json interop fields consumer, I need to invoke json string from field through a stable signature so the core json interop fields workflow remains explicit and composable. @fn inline FString JsonStringFromField( const TSharedPtr<FJsonObject> &Object, const FString &FieldName, const FString &DefaultValue = TEXT("{}")) */
inline FString JsonStringFromField(
    const TSharedPtr<FJsonObject> &Object, const FString &FieldName,
    const FString &DefaultValue = TEXT("{}")) {
  return !Object.IsValid() || !Object->HasField(FieldName)
             ? DefaultValue
             : [&]() {
                 const TSharedPtr<FJsonValue> Value =
                     Object->TryGetField(FieldName);
                 return !Value.IsValid() || Value->Type == EJson::Null
                            ? DefaultValue
                            : Value->Type == EJson::String
                                  ? Value->AsString()
                                  : StringifyValue(Value);
               }();
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
          ParseJsonObject(JsonString, JsonObject) && JsonObject.IsValid()
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
