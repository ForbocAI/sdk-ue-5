#pragma once

#include "Features/API/Endpoints/NPC/Process/NPCProcessAdapters.h"

namespace APISlice::Detail {

inline FString EncodeDirectiveRequest(const FDirectiveRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(TEXT("observation"), Request.Observation),
          Root->SetObjectField(TEXT("npcState"),
                               JsonInterop::StateToObject(Request.NpcState)),
          JsonInterop::SetFieldFromJsonString(Root, TEXT("context"),
                                              Request.ContextJson, false),
          ToJsonString(Root));
}

/**
 * Decodes a directive response into the memory-recall instruction payload.
 * User Story: As directive endpoint callers, I need typed response decoding so
 * recall query, limit, and threshold values can drive the next lookup step.
 */
inline bool DecodeDirectiveResponse(const FString &Json,
                                    FDirectiveResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
          !Root->HasTypedField<EJson::Object>(TEXT("memoryRecall")))
             ? false
             : [&]() -> bool {
                 const TSharedPtr<FJsonObject> Recall =
                     Root->GetObjectField(TEXT("memoryRecall"));
                 return (
                     Response.recallMemory.Query =
                         Recall->GetStringField(TEXT("query")),
                     Response.recallMemory.Limit =
                         JsonInterop::detail::TryGetNumberAs<int32>(
                             Recall, TEXT("limit"),
                             Response.recallMemory.Limit),
                     Response.recallMemory.Threshold =
                         JsonInterop::detail::TryGetNumberAs<float>(
                             Recall, TEXT("threshold"),
                             Response.recallMemory.Threshold),
                     true);
               }();
}

/**
 * Decodes a verdict response into a typed verdict result.
 * User Story: As verdict endpoint callers, I need response decoding so
 * validity, dialogue, actions, and memory updates return to runtime code.
 */
inline bool DecodeVerdictResponse(const FString &Json,
                                  FVerdictResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Response.bValid = JsonInterop::detail::TryGetBoolAs(
                    Root, TEXT("valid"), Response.bValid),
                Response.Signature = JsonInterop::OptionalStringFromField(
                    Root, TEXT("signature")),
                Response.StateDelta =
                    JsonInterop::StateFromField(Root, TEXT("stateDelta")),
                Response.Dialogue = Root->GetStringField(TEXT("dialogue")),
                Response.bHasAction =
                    Root->HasTypedField<EJson::Object>(TEXT("action")),
                Response.bHasAction
                    ? (Response.Action = JsonInterop::ActionFromObject(
                           Root->GetObjectField(TEXT("action"))),
                       void())
                    : void(),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *MemoryValues = nullptr;
                  (Root->TryGetArrayField(TEXT("memoryStore"), MemoryValues) &&
                   MemoryValues)
                      ? (Response.storeMemory.Empty(MemoryValues->Num()),
                         detail::ExtractMemoryStoreInstructionsRecursive(
                             *MemoryValues, Response.storeMemory, 0),
                         void())
                      : void();
                }(),
                true);
}

/**
 * Encodes a bridge-validation request into JSON text.
 * User Story: As bridge-rule validation callers, I need a request encoder so
 * action and validation context values are serialized consistently.
 */
inline FString
EncodeBridgeValidateRequest(const FBridgeValidateRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetObjectField(TEXT("action"),
                               JsonInterop::ActionToObject(Request.Action)),
          Root->SetObjectField(
              TEXT("context"),
              JsonInterop::ValidationContextToObject(Request.Context)),
          ToJsonString(Root));
}

/**
 * Decodes a validation response into a typed validation result.
 * User Story: As bridge-rule validation callers, I need one decoder so both
 * wrapped and direct validation payloads map into the same runtime type.
 */
inline bool DecodeValidationResult(const FString &Json,
                                   FValidationResult &Result) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (Result = Root->HasTypedField<EJson::Object>(TEXT("brResult"))
                             ? JsonInterop::ValidationResultFromObject(
                                   Root->GetObjectField(TEXT("brResult")))
                             : JsonInterop::ValidationResultFromObject(Root),
                true);
}

} // namespace APISlice::Detail
