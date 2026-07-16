#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Contracts/ContractsTypes.h"
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

struct FRuntimeState;

namespace APISlice {

using namespace rtk;

extern rtk::Api<FRuntimeState> api;

namespace Detail {

namespace detail {

/**
 * Recursively extracts valid strings from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 * @fn inline void ExtractStringValuesRecursive( const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FString> &Out, int32 Index)
 */
inline void ExtractStringValuesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FString> &Out,
    int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type != EJson::Null)
             ? (Out.Add(Source[Index]->AsString()), void())
             : void(),
         ExtractStringValuesRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively extracts bridge rules from a JSON value array.
 * Forward-declared; defined after DecodeBridgeRuleObject.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 * @fn inline void ExtractBridgeRulesRecursive( const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FBridgeRule> &Out, int32 Index)
 */
inline void ExtractBridgeRulesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FBridgeRule> &Out,
    int32 Index);

/**
 * Recursively extracts directive rulesets from a JSON value array.
 * Forward-declared; defined after DecodeDirectiveRuleSetObject.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 * @fn inline void ExtractDirectiveRuleSetsRecursive( const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FDirectiveRuleSet> &Out, int32 Index)
 */
inline void ExtractDirectiveRuleSetsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FDirectiveRuleSet> &Out, int32 Index);

} // namespace detail

/**
 * Serializes a UStruct-like value into JSON text.
 * User Story: As API request builders, I need a generic JSON encoder so typed
 * payloads can be posted without handwritten serialization per endpoint.
 * @fn template <typename T> inline FString ToJson(const T &Value)
 */
template <typename T> inline FString ToJson(const T &Value) {
  FString Json;
  FJsonObjectConverter::UStructToJsonObjectString(Value, Json);
  return Json;
}

/**
 * URL-encodes a value for safe path and query usage.
 * User Story: As endpoint builders, I need path-safe encoding so ids and other
 * dynamic values can be inserted into API URLs without corruption.
 * @fn inline FString Encode(const FString &Value)
 */
inline FString Encode(const FString &Value) {
  return FGenericPlatformHttp::UrlEncode(Value);
}

/**
 * Serializes a JSON object into text.
 * User Story: As codec helpers, I need object-to-string conversion so ad hoc
 * payloads can move through RTK Query request builders unchanged.
 * @fn inline FString ToJsonString(const TSharedRef<FJsonObject> &Object)
 */
inline FString ToJsonString(const TSharedRef<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  FJsonSerializer::Serialize(Object, Writer);
  return Json;
}

} // namespace Detail
} // namespace APISlice
