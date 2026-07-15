#pragma once

#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/Protocol/Handlers/HandlersAdapters.h"

namespace rtk {

namespace detail {

/**
 * Normalizes empty JSON payloads to an empty object literal.
 * User Story: As serializer helpers, I need empty payloads normalized so
 * outbound JSON contracts never receive blank strings unexpectedly.
 */
inline FString SafeJson(const FString &Json) {
  return Json.IsEmpty() ? TEXT("{}") : Json;
}

/**
 * Extracts the state JSON payload and normalizes empty values.
 * User Story: As protocol serializers, I need state payloads normalized so
 * downstream requests always receive valid JSON text.
 */
inline FString SafeStateJson(const FAgentState &State) {
  return SafeJson(State.JsonData);
}

/**
 * Reports whether the state carries a non-empty JSON payload.
 * User Story: As request builders, I need to detect meaningful state payloads
 * so I only serialize optional state when there is real data to send.
 */
inline bool HasStatePayload(const FAgentState &State) {
  return !State.JsonData.IsEmpty() && State.JsonData != TEXT("{}");
}

/**
 * Serializes a JSON object pointer into a string payload.
 * User Story: As protocol serializers, I need a shared object-to-string helper
 * so JSON payload generation stays consistent across instructions.
 */
inline FString JsonObjectToString(const TSharedPtr<FJsonObject> &Object) {
  return JsonInterop::StringifyObject(Object);
}

/**
 * Serializes an identify-actor result payload for protocol tooling.
 * User Story: As protocol execution, I need actor-identification results
 * wrapped in a stable JSON envelope so later instructions can consume them.
 */
inline FString SerializeIdentifyActorResult(const FNPCActorInfo &Actor) {
  const TSharedPtr<FJsonObject> ActorObject = MakeShared<FJsonObject>();
  ActorObject->SetStringField(TEXT("npcId"), Actor.NpcId);
  ActorObject->SetStringField(TEXT("persona"), Actor.Persona);
  ActorObject->SetObjectField(TEXT("structuredPersona"),
                              JsonInterop::StructuredPersonaToObject(
                                  Actor.Persona));
  ActorObject->SetObjectField(TEXT("data"),
                              JsonInterop::StateToObject(Actor.Data));

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("IdentifyActorResult"));
  Root->SetObjectField(TEXT("actor"), ActorObject);
  return JsonObjectToString(Root);
}

/**
 * Serializes a decision intent into a decision result payload.
 * User Story: As protocol execution, I need decision intent wrapped in a
 * stable JSON envelope so later instructions can consume it consistently.
 */
inline FString SerializeDecisionResult(const FString &Goal,
                                       const FString &ActionType,
                                       const FString &Target = TEXT("")) {
  const TSharedPtr<FJsonObject> IntentObject = MakeShared<FJsonObject>();
  IntentObject->SetStringField(TEXT("goal"), Goal);
  IntentObject->SetStringField(TEXT("actionType"), ActionType);
  !Target.IsEmpty()
      ? (IntentObject->SetStringField(TEXT("target"), Target), void())
      : void();

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("Decision"));
  Root->SetObjectField(TEXT("decisionIntent"), IntentObject);
  return JsonObjectToString(Root);
}

/**
 * Serializes reasoning output into a reasoning result payload.
 * User Story: As protocol execution, I need reasoning output wrapped in a
 * stable JSON envelope so later instructions can consume it consistently.
 */
inline FString SerializeReasoningResult(const FString &ReasoningText,
                                        const FString &ResponseText) {
  const TSharedPtr<FJsonObject> ReasoningObject = MakeShared<FJsonObject>();
  ReasoningObject->SetStringField(TEXT("reasoningText"), ReasoningText);
  ReasoningObject->SetStringField(TEXT("responseText"), ResponseText);

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("Reasoning"));
  Root->SetObjectField(TEXT("reasoningOutput"), ReasoningObject);
  return JsonObjectToString(Root);
}

/**
 * Recursively serializes memory items into a JSON value array.
 * User Story: As protocol memory serialization, I need recursive array
 * building so query-vector result payloads stay expression-style.
 */
inline void SerializeMemoryItemsRecursive(
    const TArray<FMemoryItem> &Memories,
    TArray<TSharedPtr<FJsonValue>> &Out, int32 Index) {
  Index < Memories.Num()
      ? [&]() {
          const FMemoryItem &Memory = Memories[Index];
          const TSharedPtr<FJsonObject> MemoryObject =
              MakeShared<FJsonObject>();
          MemoryObject->SetStringField(TEXT("text"), Memory.Text);
          MemoryObject->SetStringField(TEXT("type"), Memory.Type);
          MemoryObject->SetNumberField(TEXT("importance"), Memory.Importance);
          MemoryObject->SetNumberField(TEXT("similarity"), Memory.Similarity);
          Out.Add(MakeShared<FJsonValueObject>(MemoryObject));
          SerializeMemoryItemsRecursive(Memories, Out, Index + 1);
        }()
      : void();
}

/**
 * Serializes recalled memories into a query-vector result payload.
 * User Story: As protocol execution, I need recalled memories wrapped in a
 * stable JSON envelope so follow-up instructions can read them consistently.
 */
inline FString SerializeQueryVectorResult(
    const TArray<FMemoryItem> &Memories) {
  TArray<TSharedPtr<FJsonValue>> MemoryValues;
  SerializeMemoryItemsRecursive(Memories, MemoryValues, 0);

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(TEXT("type"), TEXT("QueryVectorResult"));
  Root->SetArrayField(TEXT("memories"), MemoryValues);
  return JsonObjectToString(Root);
}

/**
 * Converts a protocol instruction into the public agent response shape.
 * User Story: As protocol callers, I need typed instructions converted into
 * agent responses so runtime code can consume dialogue and actions directly.
 */
inline FAgentResponse BuildAgentResponse(const FNPCInstruction &Instruction) {
  FAgentResponse Response;
  return (Response.Dialogue = Instruction.Dialogue,
          Response.Thought = Instruction.Dialogue,
          Instruction.bHasAction
              ? (Response.Action = Instruction.Action, void())
              : void(),
          Response);
}

/**
 * Converts a single memory item into its recalled counterpart.
 * User Story: As protocol memory flows, I need item conversion so recalled
 * memory arrays can be built without imperative loop code.
 */
inline FRecalledMemory MemoryItemToRecalled(const FMemoryItem &Item) {
  FRecalledMemory M;
  M.Text = Item.Text;
  M.Type = Item.Type;
  M.Importance = Item.Importance;
  M.Similarity = Item.Similarity;
  return M;
}

/**
 * Recursively populates a recalled-memory array from memory items.
 * User Story: As protocol memory flows, I need recursive array building so
 * memory conversion stays expression-style without imperative loops.
 */
inline TArray<FRecalledMemory>
PopulateRecalledMemoriesRecursive(const TArray<FMemoryItem> &Memories,
                                  int32 Index,
                                  TArray<FRecalledMemory> Result) {
  return Index >= Memories.Num()
             ? Result
             : (Result.Add(MemoryItemToRecalled(Memories[Index])),
                PopulateRecalledMemoriesRecursive(Memories, Index + 1,
                                                  MoveTemp(Result)));
}

} // namespace detail
} // namespace rtk
