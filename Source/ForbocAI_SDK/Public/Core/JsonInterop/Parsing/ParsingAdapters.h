#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace JsonInterop {
namespace detail {

/** User Story: As a core json interop parsing consumer, I need to invoke set if non empty through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline void SetIfNonEmpty(const TSharedRef<FJsonObject> &Object, const FString &FieldName, const FString &Value) */
inline void SetIfNonEmpty(const TSharedRef<FJsonObject> &Object,
                          const FString &FieldName, const FString &Value) {
  !Value.IsEmpty() ? (Object->SetStringField(FieldName, Value), void())
                   : void();
}

/** User Story: As a core json interop parsing consumer, I need to invoke try get number as through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn template <typename T> inline T TryGetNumberAs(const TSharedPtr<FJsonObject> &Object, const FString &FieldName, T Default) */
template <typename T>
inline T TryGetNumberAs(const TSharedPtr<FJsonObject> &Object,
                        const FString &FieldName, T Default) {
  double Value = static_cast<double>(Default);
  return Object->TryGetNumberField(FieldName, Value) ? static_cast<T>(Value)
                                                     : Default;
}

/** User Story: As a core json interop parsing consumer, I need to invoke try get bool as through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline bool TryGetBoolAs(const TSharedPtr<FJsonObject> &Object, const FString &FieldName, bool Default) */
inline bool TryGetBoolAs(const TSharedPtr<FJsonObject> &Object,
                         const FString &FieldName, bool Default) {
  bool Value = Default;
  return Object->TryGetBoolField(FieldName, Value) ? Value : Default;
}

} // namespace detail

/** User Story: As a core json interop parsing consumer, I need to invoke parse json object through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline bool ParseJsonObject(const FString &Json, TSharedPtr<FJsonObject> &OutObject) */
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

/** User Story: As a core json interop parsing consumer, I need to invoke parse json array through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline bool ParseJsonArray(const FString &Json, TArray<TSharedPtr<FJsonValue>> &OutArray) */
inline bool ParseJsonArray(const FString &Json,
                           TArray<TSharedPtr<FJsonValue>> &OutArray) {
  const TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Json);
  return FJsonSerializer::Deserialize(Reader, OutArray);
}

/** User Story: As a core json interop parsing consumer, I need to invoke stringify object through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline FString StringifyObject(const TSharedPtr<FJsonObject> &Object) */
inline FString StringifyObject(const TSharedPtr<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  return Object.IsValid()
             ? (FJsonSerializer::Serialize(Object.ToSharedRef(), Writer), Json)
             : (Writer->WriteObjectStart(), Writer->WriteObjectEnd(),
                Writer->Close(), Json);
}

/** User Story: As a cross-host JSON consumer, I need objects serialized without presentation whitespace so equivalent TS and UE values share one observable wire form. @fn inline FString StringifyObjectCompact(const TSharedPtr<FJsonObject> &Object) */
inline FString
StringifyObjectCompact(const TSharedPtr<FJsonObject> &Object) {
  FString Json;
  const auto Writer =
      TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(
          &Json);
  return Object.IsValid()
             ? (FJsonSerializer::Serialize(Object.ToSharedRef(), Writer), Json)
             : (Writer->WriteObjectStart(), Writer->WriteObjectEnd(),
                Writer->Close(), Json);
}

/**
 * User Story: As a core json interop parsing consumer, I need to invoke string array values through a stable signature so the core json interop parsing workflow remains explicit and composable.
 * @fn inline TArray<TSharedPtr<FJsonValue>> StringArrayValues(const TArray<FString> &Values, int32 Index = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, TArray<TSharedPtr<FJsonValue>> Result = {})
 */
inline TArray<TSharedPtr<FJsonValue>>
StringArrayValues(const TArray<FString> &Values, int32 Index = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA,
                  TArray<TSharedPtr<FJsonValue>> Result = {}) {
  return Index >= Values.Num()
             ? Result
             : (Result.Add(MakeShared<FJsonValueString>(Values[Index])),
                StringArrayValues(Values, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, MoveTemp(Result)));
}

/** User Story: As a core json interop parsing consumer, I need to invoke set string array field through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline void SetStringArrayField(const TSharedRef<FJsonObject> &Object, const FString &FieldName, const TArray<FString> &Values) */
inline void SetStringArrayField(const TSharedRef<FJsonObject> &Object,
                                const FString &FieldName,
                                const TArray<FString> &Values) {
  Object->SetArrayField(FieldName, StringArrayValues(Values));
}

/** User Story: As a core json interop parsing consumer, I need to invoke stringify array through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline FString StringifyArray( const TArray<TSharedPtr<FJsonValue>> &Array) */
inline FString StringifyArray(
    const TArray<TSharedPtr<FJsonValue>> &Array) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  FJsonSerializer::Serialize(Array, Writer);
  return Json;
}

/** User Story: As a core json interop parsing consumer, I need to invoke stringify value through a stable signature so the core json interop parsing workflow remains explicit and composable. @fn inline FString StringifyValue(const TSharedPtr<FJsonValue> &Value) */
inline FString StringifyValue(const TSharedPtr<FJsonValue> &Value) {
  return !Value.IsValid()
             ? FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV529CBDAF6B80))
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
                                 return FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA816DD577B9B),
                                                        *Value->AsString());
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Boolean),
                               [&Value](const EJson &) {
                                 return Value->AsBool()
                                            ? FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDD287442E709))
                                            : FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFD3116F7FE2C));
                               }),
                           func::when<EJson, FString>(
                               func::equals<EJson>(EJson::Number),
                               [&Value](const EJson &) {
                                 return FString::SanitizeFloat(
                                     Value->AsNumber());
                               }),
                       }),
                   FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV529CBDAF6B80)));
}

} // namespace JsonInterop
