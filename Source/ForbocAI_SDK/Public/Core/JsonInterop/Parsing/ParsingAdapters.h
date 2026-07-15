#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace JsonInterop {
namespace detail {

inline void SetIfNonEmpty(const TSharedRef<FJsonObject> &Object,
                          const FString &FieldName, const FString &Value) {
  !Value.IsEmpty() ? (Object->SetStringField(FieldName, Value), void())
                   : void();
}

template <typename T>
inline T TryGetNumberAs(const TSharedPtr<FJsonObject> &Object,
                        const FString &FieldName, T Default) {
  double Value = static_cast<double>(Default);
  return Object->TryGetNumberField(FieldName, Value) ? static_cast<T>(Value)
                                                     : Default;
}

inline bool TryGetBoolAs(const TSharedPtr<FJsonObject> &Object,
                         const FString &FieldName, bool Default) {
  bool Value = Default;
  return Object->TryGetBoolField(FieldName, Value) ? Value : Default;
}

} // namespace detail

inline bool ParseJsonObject(const FString &Json,
                            TSharedPtr<FJsonObject> &OutObject) {
  return Json.IsEmpty()
             ? (OutObject = MakeShared<FJsonObject>(), true)
             : [&]() {
                 const TSharedRef<TJsonReader<>> Reader =
                     TJsonReaderFactory<>::Create(Json);
                 return FJsonSerializer::Deserialize(Reader, OutObject) &&
                                OutObject.IsValid()
                            ? true
                            : (OutObject = MakeShared<FJsonObject>(), false);
               }();
}

inline bool ParseJsonArray(const FString &Json,
                           TArray<TSharedPtr<FJsonValue>> &OutArray) {
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
  return FJsonSerializer::Deserialize(Reader, OutArray);
}

inline FString StringifyObject(const TSharedPtr<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  return Object.IsValid()
             ? (FJsonSerializer::Serialize(Object.ToSharedRef(), Writer), Json)
             : (Writer->WriteObjectStart(), Writer->WriteObjectEnd(),
                Writer->Close(), Json);
}

inline TArray<TSharedPtr<FJsonValue>>
StringArrayValues(const TArray<FString> &Values, int32 Index = 0,
                  TArray<TSharedPtr<FJsonValue>> Result = {}) {
  return Index >= Values.Num()
             ? Result
             : (Result.Add(MakeShared<FJsonValueString>(Values[Index])),
                StringArrayValues(Values, Index + 1, MoveTemp(Result)));
}

inline void SetStringArrayField(const TSharedRef<FJsonObject> &Object,
                                const FString &FieldName,
                                const TArray<FString> &Values) {
  Object->SetArrayField(FieldName, StringArrayValues(Values));
}

inline FString StringifyArray(
    const TArray<TSharedPtr<FJsonValue>> &Array) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  FJsonSerializer::Serialize(Array, Writer);
  return Json;
}

inline FString StringifyValue(const TSharedPtr<FJsonValue> &Value) {
  return !Value.IsValid()
             ? FString(TEXT("null"))
             : func::or_else(
                   func::multi_match<EJson, FString>(
                       Value->Type,
                       {
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Object),
                               [&Value](const EJson &) {
                                 return StringifyObject(Value->AsObject());
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Array),
                               [&Value](const EJson &) {
                                 return StringifyArray(Value->AsArray());
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::String),
                               [&Value](const EJson &) {
                                 return FString::Printf(TEXT("\"%s\""),
                                                        *Value->AsString());
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Boolean),
                               [&Value](const EJson &) {
                                 return Value->AsBool()
                                            ? FString(TEXT("true"))
                                            : FString(TEXT("false"));
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Number),
                               [&Value](const EJson &) {
                                 return FString::SanitizeFloat(
                                     Value->AsNumber());
                               }),
                       }),
                   FString(TEXT("null")));
}

} // namespace JsonInterop
