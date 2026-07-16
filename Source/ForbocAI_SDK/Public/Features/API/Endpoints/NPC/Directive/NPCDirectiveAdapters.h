#pragma once

#include "Features/API/Endpoints/NPC/Process/NPCProcessAdapters.h"

namespace APISlice::Detail {

/** User Story: As a endpoints npc directive consumer, I need to invoke encode directive request through a stable signature so the endpoints npc directive workflow remains explicit and composable. @fn inline FString EncodeDirectiveRequest(const FDirectiveRequest &Request) */
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
 * @fn inline bool DecodeDirectiveResponse(const FString &Json, FDirectiveResponse &Response)
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
 * @fn inline bool DecodeVerdictResponse(const FString &Json, FVerdictResponse &Response)
 */
inline bool DecodeVerdictResponse(const FString &Json,
                                  FVerdictResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  const TArray<TSharedPtr<FJsonValue>> *MemoryValues = nullptr;
  const bool bValid =
      JsonInterop::ParseJsonObject(Json, Root) && Root.IsValid() &&
      Root->HasTypedField<EJson::Boolean>(TEXT("valid")) &&
      JsonInterop::HasOptionalFieldType(Root, TEXT("signature"),
                                        EJson::String) &&
      Root->HasTypedField<EJson::Array>(TEXT("memoryStore")) &&
      Root->TryGetArrayField(TEXT("memoryStore"), MemoryValues) &&
      MemoryValues &&
      Root->HasTypedField<EJson::Object>(TEXT("stateDelta")) &&
      JsonInterop::HasOptionalFieldType(Root, TEXT("action"), EJson::Object) &&
      Root->HasTypedField<EJson::String>(TEXT("dialogue"));
  return !bValid
             ? false
             : [&]() {
  const bool bHasAction = JsonInterop::HasNonNullField(Root, TEXT("action"));
  const func::Maybe<FAgentAction> Action =
      bHasAction
          ? JsonInterop::DecodeActionObject(Root->GetObjectField(TEXT("action")))
          : func::nothing<FAgentAction>();
  const func::Maybe<TArray<FMemoryStoreInstruction>> Memories =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                 FMemoryStoreInstruction>(
          *MemoryValues, JsonInterop::DecodeMemoryStoreInstructionValue);
  const bool bDecoded =
      (!bHasAction || func::is_just(Action)) && func::is_just(Memories);
  return !bDecoded
             ? false
             : [&]() {
                 FVerdictResponse Decoded;
                 Decoded.bValid = Root->GetBoolField(TEXT("valid"));
                 Decoded.Signature = JsonInterop::OptionalStringFromField(
                     Root, TEXT("signature"));
                 Decoded.StateDelta =
                     JsonInterop::StateFromField(Root, TEXT("stateDelta"));
                 Decoded.Dialogue = Root->GetStringField(TEXT("dialogue"));
                 Decoded.bHasAction = bHasAction;
                 Decoded.Action = func::or_else(Action, FAgentAction());
                 Decoded.storeMemory = func::or_else(
                     Memories, TArray<FMemoryStoreInstruction>());
                 Response = Decoded;
                 return true;
               }();
               }();
}

/**
 * Encodes a bridge-validation request into JSON text.
 * User Story: As bridge-rule validation callers, I need a request encoder so
 * action and validation context values are serialized consistently.
 * @fn inline FString EncodeBridgeValidateRequest(const FBridgeValidateRequest &Request)
 */
inline FString
EncodeBridgeValidateRequest(const FBridgeValidateRequest &Request) {
  const JsonInterop::BridgeConfiguration::FBridgeSerializationData &Data =
      JsonInterop::BridgeConfiguration::bridgeSerializationData();
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetObjectField(Data.ValidateRequest.Action,
                               JsonInterop::ActionToObject(Request.Action)),
          Root->SetObjectField(
              Data.ValidateRequest.Context,
              JsonInterop::ValidationContextToObject(Request.Context)),
          ToJsonString(Root));
}

/**
 * Decodes a validation response into a typed validation result.
 * User Story: As bridge-rule validation callers, I need only the canonical
 * direct validation payload accepted so API drift fails at the boundary.
 * @fn inline bool DecodeValidationResult(const FString &Json, FValidationResult &Result)
 */
inline bool DecodeValidationResult(const FString &Json,
                                   FValidationResult &Result) {
  TSharedPtr<FJsonObject> Root;
  return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid()
             ? false
             : func::match(
                   JsonInterop::DecodeValidationResultObject(Root),
                   [&Result](const FValidationResult &Decoded) {
                     Result = Decoded;
                     return true;
                   },
                   []() { return false; });
}

} // namespace APISlice::Detail
