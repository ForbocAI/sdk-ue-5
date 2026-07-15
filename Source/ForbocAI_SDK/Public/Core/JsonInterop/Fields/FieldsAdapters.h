#pragma once

#include "Core/JsonInterop/Parsing/ParsingAdapters.h"

namespace JsonInterop {

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

inline TSharedPtr<FJsonObject> ParseJsonObjectOrEmpty(const FString &Json) {
  TSharedPtr<FJsonObject> Object;
  ParseJsonObject(Json, Object);
  return Object.IsValid() ? Object : MakeShared<FJsonObject>();
}

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
