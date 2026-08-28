#pragma once

#include "Core/fp.hpp"
#include "Systems/API/Endpoints/NPC/Process/Configuration/ProcessConfigurationAdapters.h"
#include "Systems/API/Serialization/APISerializationAdapters.h"
#include "Systems/API/Serialization/Memory/Configuration/SerializationMemoryConfigurationAdapters.h"
#include "Systems/Protocol/Handlers/HandlersAdapters.h"

namespace rtk {

namespace detail {

/**
 * Reports whether the state carries a non-empty JSON payload.
 * User Story: As request builders, I need to detect meaningful state payloads
 * so I only serialize optional state when there is real data to send.
 * @fn inline bool HasStatePayload(const FAgentState &State)
 */
inline bool HasStatePayload(const FAgentState &State) {
  return !State.JsonData.IsEmpty();
}

/**
 * Serializes a JSON object pointer into a string payload.
 * User Story: As protocol serializers, I need a shared object-to-string helper
 * so JSON payload generation stays consistent across instructions.
 * @fn inline FString JsonObjectToString(const TSharedPtr<FJsonObject> &Object)
 */
inline FString JsonObjectToString(const TSharedPtr<FJsonObject> &Object) {
  return JsonInterop::StringifyObject(Object);
}

/**
 * Serializes one memory item through the authored recalled-memory contract.
 * User Story: As protocol memory serialization, I need a pure item adapter so
 * query-vector arrays can be mapped without index mutation.
 * @fn inline TSharedPtr<FJsonValue> MemoryItemToJsonValue(const FMemoryItem &Memory)
 */
inline TSharedPtr<FJsonValue>
MemoryItemToJsonValue(const FMemoryItem &Memory) {
  const auto &Fields =
      JsonInterop::MemoryConfiguration::memoryContractData().RecalledMemory;
  const TSharedPtr<FJsonObject> Object = MakeShared<FJsonObject>();
  Object->SetStringField(Fields.Text, Memory.Text);
  Object->SetStringField(Fields.Type, Memory.Type);
  Object->SetNumberField(Fields.Importance, Memory.Importance);
  Object->SetNumberField(Fields.Similarity, Memory.Similarity);
  return MakeShared<FJsonValueObject>(Object);
}

/**
 * Serializes recalled memories into a query-vector result payload.
 * User Story: As protocol execution, I need recalled memories wrapped in a
 * stable JSON envelope so follow-up instructions can read them consistently.
 * @fn inline FString SerializeQueryVectorResult( const TArray<FMemoryItem> &Memories)
 */
inline FString SerializeQueryVectorResult(
    const TArray<FMemoryItem> &Memories) {
  const auto &Data =
      APISlice::NPCProcessConfiguration::processContractData();
  const TArray<TSharedPtr<FJsonValue>> MemoryValues =
      func::map_array<FMemoryItem, TSharedPtr<FJsonValue>>(
          Memories, MemoryItemToJsonValue);

  const TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetStringField(Data.Instruction.Type, Data.ResultTypes.QueryVector);
  Root->SetArrayField(Data.Tape.Memories, MemoryValues);
  return JsonObjectToString(Root);
}

/**
 * Converts a protocol instruction into the public agent response shape.
 * User Story: As protocol callers, I need typed instructions converted into
 * agent responses so runtime code can consume dialogue and actions directly.
 * @fn inline FAgentResponse BuildAgentResponse(const FNPCInstruction &Instruction, const FNPCProcessTape &Tape)
 */
inline FAgentResponse BuildAgentResponse(const FNPCInstruction &Instruction,
                                         const FNPCProcessTape &Tape) {
  FAgentResponse Response;
  return (Response.Dialogue = Instruction.Dialogue,
          Response.bHasAction = Instruction.bHasAction,
          Instruction.bHasAction
              ? (Response.Action = Instruction.Action, void())
              : void(),
          Response.bHasThoughtResult = Tape.bHasDecisionIntent,
          Tape.bHasDecisionIntent
              ? (Response.ThoughtResult = Tape.DecisionIntent, void())
              : void(),
          Response.bHasReasoningResult = Tape.bHasReasoningOutput,
          Tape.bHasReasoningOutput
              ? (Response.ReasoningResult = Tape.ReasoningOutput, void())
              : void(),
          Response.bHasPrompt = Tape.bHasPrompt,
          Tape.bHasPrompt ? (Response.Prompt = Tape.Prompt, void()) : void(),
          Response);
}

/**
 * Converts a single memory item into its recalled counterpart.
 * User Story: As protocol memory flows, I need item conversion so recalled
 * memory arrays can be built without imperative loop code.
 * @fn inline FRecalledMemory MemoryItemToRecalled(const FMemoryItem &Item)
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
 * Maps memory items into their recalled-memory counterparts.
 * User Story: As protocol memory flows, I need pure array mapping so tape
 * updates preserve the SDK memory result without mutation-oriented loops.
 * @fn inline TArray<FRecalledMemory> MemoryItemsToRecalled(const TArray<FMemoryItem> &Memories)
 */
inline TArray<FRecalledMemory>
MemoryItemsToRecalled(const TArray<FMemoryItem> &Memories) {
  return func::map_array<FMemoryItem, FRecalledMemory>(Memories,
                                                        MemoryItemToRecalled);
}

} // namespace detail
} // namespace rtk
