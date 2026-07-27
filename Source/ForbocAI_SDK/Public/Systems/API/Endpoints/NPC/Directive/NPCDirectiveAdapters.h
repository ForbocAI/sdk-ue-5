#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/API/Endpoints/NPC/Process/NPCProcessAdapters.h"

namespace APISlice::Detail {

/** User Story: As a endpoints npc directive consumer, I need to invoke encode directive request through a stable signature so the endpoints npc directive workflow remains explicit and composable. @fn inline FString EncodeDirectiveRequest(const FDirectiveRequest &Request) */
inline FString EncodeDirectiveRequest(const FDirectiveRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90F5E365EC02), Request.Observation),
          Root->SetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV34492EFB9466),
                               JsonInterop::StateToObject(Request.NpcState)),
          JsonInterop::SetFieldFromJsonString(Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0E38DAE2D51F),
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
          !Root->HasTypedField<EJson::Object>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0B8202B704D1)))
             ? false
             : [&]() -> bool {
                 const TSharedPtr<FJsonObject> Recall =
                     Root->GetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0B8202B704D1));
                 return (
                     Response.recallMemory.Query =
                         Recall->GetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV32006DA77DEF)),
                     Response.recallMemory.Limit =
                         JsonInterop::detail::TryGetNumberAs<int32>(
                             Recall, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD5BFB13FEF4E),
                             Response.recallMemory.Limit),
                     Response.recallMemory.Threshold =
                         JsonInterop::detail::TryGetNumberAs<float>(
                             Recall, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBA571F023F22),
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
      Root->HasTypedField<EJson::Boolean>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVAF6F6A1B5175)) &&
      JsonInterop::HasOptionalFieldType(Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB677912989D9),
                                        EJson::String) &&
      Root->HasTypedField<EJson::Array>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV28CCA9591372)) &&
      Root->TryGetArrayField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV28CCA9591372), MemoryValues) &&
      MemoryValues &&
      Root->HasTypedField<EJson::Object>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV77DF615E27E6)) &&
      JsonInterop::HasOptionalFieldType(Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF673571534B7), EJson::Object) &&
      Root->HasTypedField<EJson::String>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDEE4B8D4AE2B));
  return !bValid
             ? false
             : [&]() {
  const bool bHasAction = JsonInterop::HasNonNullField(Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF673571534B7));
  const func::Maybe<FAgentAction> Action =
      bHasAction
          ? JsonInterop::DecodeActionObject(Root->GetObjectField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF673571534B7)))
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
                 Decoded.bValid = Root->GetBoolField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVAF6F6A1B5175));
                 Decoded.Signature = JsonInterop::OptionalStringFromField(
                     Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB677912989D9));
                 Decoded.StateDelta =
                     JsonInterop::StateFromField(Root, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV77DF615E27E6));
                 Decoded.Dialogue = Root->GetStringField(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDEE4B8D4AE2B));
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
