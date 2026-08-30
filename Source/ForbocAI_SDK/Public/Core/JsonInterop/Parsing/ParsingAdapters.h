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
                 const bool bParsed =
                     FJsonSerializer::Deserialize(Reader, OutObject);
                 return func::match(
                     func::fromNullable(OutObject,
                                        bParsed && OutObject.Get()),
                     [](const TSharedPtr<FJsonObject> &) { return true; },
                     [&OutObject]() {
                       OutObject = MakeShared<FJsonObject>();
                       return false;
                     });
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
  return func::match(
      func::fromNullable(Object, static_cast<bool>(Object.Get())),
      [&Writer, &Json](const TSharedPtr<FJsonObject> &Present) {
        FJsonSerializer::Serialize(Present.ToSharedRef(), Writer);
        return Json;
      },
      [&Writer, &Json]() {
        Writer->WriteObjectStart();
        Writer->WriteObjectEnd();
        Writer->Close();
        return Json;
      });
}

/** User Story: As a cross-host JSON consumer, I need objects serialized without presentation whitespace so equivalent TS and UE values share one observable wire form. @fn inline FString StringifyObjectCompact(const TSharedPtr<FJsonObject> &Object) */
inline FString
StringifyObjectCompact(const TSharedPtr<FJsonObject> &Object) {
  FString Json;
  const auto Writer =
      TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(
          &Json);
  return func::match(
      func::fromNullable(Object, static_cast<bool>(Object.Get())),
      [&Writer, &Json](const TSharedPtr<FJsonObject> &Present) {
        FJsonSerializer::Serialize(Present.ToSharedRef(), Writer);
        return Json;
      },
      [&Writer, &Json]() {
        Writer->WriteObjectStart();
        Writer->WriteObjectEnd();
        Writer->Close();
        return Json;
      });
}

/**
 * User Story: As a core json interop parsing consumer, I need to invoke string array values through a stable signature so the core json interop parsing workflow remains explicit and composable.
 * @fn inline TArray<TSharedPtr<FJsonValue>> StringArrayValues(const TArray<FString> &Values)
 */
inline TArray<TSharedPtr<FJsonValue>>
StringArrayValues(const TArray<FString> &Values) {
  return func::map_array<FString, TSharedPtr<FJsonValue>>(
      Values, [](const FString &Value) -> TSharedPtr<FJsonValue> {
        return MakeShared<FJsonValueString>(Value);
      });
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
  return func::match(
      func::fromNullable(Value, static_cast<bool>(Value.Get())),
      [](const TSharedPtr<FJsonValue> &Present) {
        return func::or_else(
            func::multi_match<EJson, FString>(
                Present->Type,
                {
                    func::when<EJson, FString>(
                        func::equals<EJson>(EJson::Object),
                        [&Present](const EJson &) {
                          return StringifyObject(Present->AsObject());
                        }),
                    func::when<EJson, FString>(
                        func::equals<EJson>(EJson::Array),
                        [&Present](const EJson &) {
                          return StringifyArray(Present->AsArray());
                        }),
                    func::when<EJson, FString>(
                        func::equals<EJson>(EJson::String),
                        [&Present](const EJson &) {
                          return FString::Printf(
                              TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA816DD577B9B),
                              *Present->AsString());
                        }),
                    func::when<EJson, FString>(
                        func::equals<EJson>(EJson::Boolean),
                        [&Present](const EJson &) {
                          return Present->AsBool()
                                     ? FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDD287442E709))
                                     : FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFD3116F7FE2C));
                        }),
                    func::when<EJson, FString>(
                        func::equals<EJson>(EJson::Number),
                        [&Present](const EJson &) {
                          return FString::SanitizeFloat(Present->AsNumber());
                        }),
                }),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV529CBDAF6B80)));
      },
      []() {
        return FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV529CBDAF6B80));
      });
}

} // namespace JsonInterop
