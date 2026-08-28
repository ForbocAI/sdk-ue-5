#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/JsonInterop.h"
#include "Core/fp.hpp"

#include <cmath>
#include <limits>

namespace APISlice::Detail {

/** User Story: As strict Ghost decoding, I need required strings accepted only with their canonical JSON type. @fn inline bool DecodeGhostStringField(const TSharedPtr<FJsonObject> &Object, const FString &Field, FString &Output) */
inline bool DecodeGhostStringField(const TSharedPtr<FJsonObject> &Object,
                                   const FString &Field, FString &Output) {
  return Object.IsValid() && Object->HasTypedField<EJson::String>(Field)
             ? (Output = Object->GetStringField(Field), true)
             : false;
}

/** User Story: As Ghost identity decoding, I need identity text rejected when it is empty after trimming. @fn inline bool DecodeGhostIdentityField(const TSharedPtr<FJsonObject> &Object, const FString &Field, int32 MinimumLength, FString &Output) */
inline bool DecodeGhostIdentityField(const TSharedPtr<FJsonObject> &Object,
                                     const FString &Field,
                                     int32 MinimumLength, FString &Output) {
  FString Value;
  return DecodeGhostStringField(Object, Field, Value) &&
                 Value.TrimStartAndEnd().Len() >= MinimumLength
             ? (Output = Value, true)
             : false;
}

/** User Story: As strict Ghost decoding, I need integer fields rejected when JSON numbers are fractional or outside the target range. @fn inline bool DecodeGhostInt32Field(const TSharedPtr<FJsonObject> &Object, const FString &Field, int32 &Output) */
inline bool DecodeGhostInt32Field(const TSharedPtr<FJsonObject> &Object,
                                  const FString &Field, int32 &Output) {
  double Value = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  const bool bValid = Object.IsValid() &&
                      Object->TryGetNumberField(Field, Value) &&
                      FMath::IsFinite(Value) &&
                      std::trunc(Value) == Value &&
                      Value >= static_cast<double>(MIN_int32) &&
                      Value <= static_cast<double>(MAX_int32);
  return bValid ? (Output = static_cast<int32>(Value), true) : false;
}

/** User Story: As strict Ghost decoding, I need duration integers preserved without fractional coercion. @fn inline bool DecodeGhostInt64Field(const TSharedPtr<FJsonObject> &Object, const FString &Field, int64 &Output) */
inline bool DecodeGhostInt64Field(const TSharedPtr<FJsonObject> &Object,
                                  const FString &Field, int64 &Output) {
  double Value = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  const bool bValid = Object.IsValid() &&
                      Object->TryGetNumberField(Field, Value) &&
                      FMath::IsFinite(Value) &&
                      std::trunc(Value) == Value &&
                      Value >= static_cast<double>(
                                   std::numeric_limits<int64>::lowest()) &&
                      Value < static_cast<double>(
                                  std::numeric_limits<int64>::max());
  return bValid ? (Output = static_cast<int64>(Value), true) : false;
}

/** User Story: As strict Ghost decoding, I need finite numeric measurements preserved without client defaults. @fn inline bool DecodeGhostFloatField(const TSharedPtr<FJsonObject> &Object, const FString &Field, float &Output) */
inline bool DecodeGhostFloatField(const TSharedPtr<FJsonObject> &Object,
                                  const FString &Field, float &Output) {
  double Value = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  return Object.IsValid() && Object->TryGetNumberField(Field, Value) &&
                 FMath::IsFinite(Value)
             ? (Output = static_cast<float>(Value), true)
             : false;
}

/**
 * User Story: As strict Ghost decoding, I need nullable optional strings distinguished from malformed values.
 * @fn inline bool DecodeGhostOptionalStringField( const TSharedPtr<FJsonObject> &Object, const FString &Field, FString &Output)
 */
inline bool DecodeGhostOptionalStringField(
    const TSharedPtr<FJsonObject> &Object, const FString &Field,
    FString &Output) {
  const bool bValid = JsonInterop::HasOptionalFieldType(
      Object, Field, EJson::String);
  return !bValid
             ? false
             : !JsonInterop::HasNonNullField(Object, Field)
                   ? (Output = FString(), true)
                   : (Output = Object->GetStringField(Field), true);
}

/** User Story: As strict Ghost array decoding, I need each dimension accepted only as a JSON string. @fn inline func::Maybe<FString> DecodeGhostStringValue(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FString>
DecodeGhostStringValue(const TSharedPtr<FJsonValue> &Value) {
  return Value.IsValid() && Value->Type == EJson::String
             ? func::just(Value->AsString())
             : func::nothing<FString>();
}

/**
 * User Story: As strict Ghost array decoding, I need all dimensions validated before any enter domain state.
 * @fn inline func::Maybe<TArray<FString>> DecodeGhostStringArrayField( const TSharedPtr<FJsonObject> &Object, const FString &Field)
 */
inline func::Maybe<TArray<FString>> DecodeGhostStringArrayField(
    const TSharedPtr<FJsonObject> &Object, const FString &Field) {
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  return !Object.IsValid() || !Object->TryGetArrayField(Field, Values) ||
                 !Values
             ? func::nothing<TArray<FString>>()
             : func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
                   *Values, DecodeGhostStringValue);
}

} // namespace APISlice::Detail
