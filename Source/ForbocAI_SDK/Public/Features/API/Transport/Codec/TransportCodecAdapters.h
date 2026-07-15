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
 * Recursively extracts recalled memories from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractRecalledMemoriesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FRecalledMemory> &Out,
    int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? (Out.Add(
                    JsonInterop::RecalledMemoryFromObject(Source[Index]->AsObject())),
                void())
             : void(),
         ExtractRecalledMemoriesRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively builds JSON value objects from recalled memories.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void BuildRecalledMemoriesRecursive(
    const TArray<FRecalledMemory> &Source,
    TArray<TSharedPtr<FJsonValue>> &Out, int32 Index) {
  Index < Source.Num()
      ? (Out.Add(MakeShared<FJsonValueObject>(
             JsonInterop::RecalledMemoryToObject(Source[Index]))),
         BuildRecalledMemoriesRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively extracts memory store instructions from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractMemoryStoreInstructionsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FMemoryStoreInstruction> &Out, int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? (Out.Add(JsonInterop::MemoryStoreInstructionFromObject(
                    Source[Index]->AsObject())),
                void())
             : void(),
         ExtractMemoryStoreInstructionsRecursive(Source, Out, Index + 1),
         void())
      : void();
}

/**
 * Recursively extracts valid strings from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
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
 */
inline void ExtractBridgeRulesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FBridgeRule> &Out,
    int32 Index);

/**
 * Recursively extracts directive rulesets from a JSON value array.
 * Forward-declared; defined after DecodeDirectiveRuleSetObject.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractDirectiveRuleSetsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FDirectiveRuleSet> &Out, int32 Index);

/**
 * Recursively extracts ghost error strings from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractGhostErrorsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FString> &Out,
    int32 Index) {
  Index < Source.Num()
      ? (Source[Index].IsValid()
             ? (Out.Add(Source[Index]->AsString()), void())
             : void(),
         ExtractGhostErrorsRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively extracts ghost test result records from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractGhostTestRecordsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FGhostResultRecord> &Out, int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? [&]() {
                 const TSharedPtr<FJsonObject> Test =
                     Source[Index]->AsObject();
                 FGhostResultRecord Record;
                 Record.TestName = JsonInterop::OptionalStringFromField(
                     Test, TEXT("testName"));
                 Record.bTestPassed = JsonInterop::detail::TryGetBoolAs(
                     Test, TEXT("testPassed"), false);
                 Record.TestDuration = JsonInterop::detail::TryGetNumberAs<int64>(
                     Test, TEXT("testDuration"), 0);
                 Record.TestError = JsonInterop::OptionalStringFromField(
                     Test, TEXT("testError"));
                 Record.TestScreenshot = JsonInterop::OptionalStringFromField(
                     Test, TEXT("testScreenshot"));
                 Out.Add(Record);
               }()
             : void(),
         ExtractGhostTestRecordsRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively extracts ghost metric pairs from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractGhostMetricPairsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TMap<FString, float> &Out,
    int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Array)
             ? [&]() {
                 const TArray<TSharedPtr<FJsonValue>> Pair =
                     Source[Index]->AsArray();
                 (Pair.Num() == 2 && Pair[0].IsValid() && Pair[1].IsValid())
                     ? (Out.Add(Pair[0]->AsString(),
                                static_cast<float>(Pair[1]->AsNumber())),
                        void())
                     : void();
               }()
             : void(),
         ExtractGhostMetricPairsRecursive(Source, Out, Index + 1), void())
      : void();
}

/**
 * Recursively extracts ghost history entries from a JSON value array.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline void ExtractGhostHistoryEntriesRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source,
    TArray<FGhostHistoryEntry> &Out, int32 Index);

/**
 * Helper to resolve a field through its primary name, then its alias.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline FString FieldOrAlias(const TSharedPtr<FJsonObject> &Object,
                            const FString &Primary,
                            const FString &Alias) {
  return Object->HasField(Primary) ? Object->GetStringField(Primary)
                                   : Object->GetStringField(Alias);
}

/**
 * Helper to try reading a number field and assigning a float.
 * Returns the read value or default.
 * User Story: As a maintainer, I need this note so the surrounding code intent
 * stays clear during maintenance and debugging.
 */
inline float TryGetPassRate(const TSharedPtr<FJsonObject> &Object,
                            const FString &Primary,
                            const FString &Alias) {
  double Value = 0.0;
  return (Object->TryGetNumberField(Primary, Value) ||
          Object->TryGetNumberField(Alias, Value))
             ? static_cast<float>(Value)
             : 0.0f;
}

} // namespace detail

/**
 * Serializes a UStruct-like value into JSON text.
 * User Story: As API request builders, I need a generic JSON encoder so typed
 * payloads can be posted without handwritten serialization per endpoint.
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
 */
inline FString Encode(const FString &Value) {
  return FGenericPlatformHttp::UrlEncode(Value);
}

/**
 * Serializes a JSON object into text.
 * User Story: As codec helpers, I need object-to-string conversion so ad hoc
 * payloads can move through RTK Query request builders unchanged.
 */
inline FString ToJsonString(const TSharedRef<FJsonObject> &Object) {
  FString Json;
  const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
  FJsonSerializer::Serialize(Object, Writer);
  return Json;
}

} // namespace Detail
} // namespace APISlice
