#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/JsonInterop.h"
#include "Systems/API/Serialization/Memory/Configuration/SerializationMemoryConfigurationAdapters.h"
#include "Components/Contracts/ContractsTypes.h"

namespace JsonInterop {

/** User Story: As a api serialization memory consumer, I need to invoke memory store instruction to object through a stable signature so the api serialization memory workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> MemoryStoreInstructionToObject(const FMemoryStoreInstruction &Instruction) */
inline TSharedRef<FJsonObject>
MemoryStoreInstructionToObject(const FMemoryStoreInstruction &Instruction) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(Data.StoreInstruction.Text, Instruction.Text),
          Object->SetStringField(Data.StoreInstruction.Type, Instruction.Type),
          Object->SetNumberField(Data.StoreInstruction.Importance,
                                 Instruction.Importance),
          Object);
}

/** User Story: As a Memory transport consumer, I need store instructions decoded only when every required field has its canonical type so partial memory persistence cannot be reported as success. @fn inline func::Maybe<FMemoryStoreInstruction> DecodeMemoryStoreInstructionObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FMemoryStoreInstruction>
DecodeMemoryStoreInstructionObject(const TSharedPtr<FJsonObject> &Object) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  double Importance = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.StoreInstruction.Text) &&
      Object->HasTypedField<EJson::String>(Data.StoreInstruction.Type) &&
      Object->TryGetNumberField(Data.StoreInstruction.Importance, Importance) &&
      FMath::IsFinite(Importance);
  return !bValid
             ? func::nothing<FMemoryStoreInstruction>()
             : [&]() {
                 FMemoryStoreInstruction Instruction;
                 Instruction.Text =
                     Object->GetStringField(Data.StoreInstruction.Text);
                 Instruction.Type =
                     Object->GetStringField(Data.StoreInstruction.Type);
                 Instruction.Importance = static_cast<float>(Importance);
                 return func::just(Instruction);
               }();
}

/** User Story: As an FP collection pipeline, I need JSON memory-store values lifted into Maybe so strict traversal rejects the complete array on the first malformed entry. @fn inline func::Maybe<FMemoryStoreInstruction> DecodeMemoryStoreInstructionValue(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FMemoryStoreInstruction>
DecodeMemoryStoreInstructionValue(const TSharedPtr<FJsonValue> &Value) {
  return !Value.IsValid() || Value->Type != EJson::Object
             ? func::nothing<FMemoryStoreInstruction>()
             : DecodeMemoryStoreInstructionObject(Value->AsObject());
}

/** User Story: As a api serialization memory consumer, I need to invoke recalled memory to object through a stable signature so the api serialization memory workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> RecalledMemoryToObject(const FRecalledMemory &Memory) */
inline TSharedRef<FJsonObject>
RecalledMemoryToObject(const FRecalledMemory &Memory) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(Data.RecalledMemory.Text, Memory.Text),
          Object->SetStringField(Data.RecalledMemory.Type, Memory.Type),
          Object->SetNumberField(Data.RecalledMemory.Importance,
                                 Memory.Importance),
          Object->SetNumberField(Data.RecalledMemory.Similarity,
                                 Memory.Similarity),
          Object);
}

/** User Story: As a Memory transport consumer, I need recalled memories decoded without coercion so malformed vector results cannot enter the protocol tape. @fn inline func::Maybe<FRecalledMemory> DecodeRecalledMemoryObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FRecalledMemory>
DecodeRecalledMemoryObject(const TSharedPtr<FJsonObject> &Object) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  double Importance = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  double Similarity = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  const bool bHasSimilarity =
      HasNonNullField(Object, Data.RecalledMemory.Similarity);
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.RecalledMemory.Text) &&
      Object->HasTypedField<EJson::String>(Data.RecalledMemory.Type) &&
      Object->TryGetNumberField(Data.RecalledMemory.Importance, Importance) &&
      FMath::IsFinite(Importance) &&
      HasOptionalFieldType(Object, Data.RecalledMemory.Similarity,
                           EJson::Number) &&
      (!bHasSimilarity ||
       (Object->TryGetNumberField(Data.RecalledMemory.Similarity, Similarity) &&
        FMath::IsFinite(Similarity)));
  return !bValid
             ? func::nothing<FRecalledMemory>()
             : [&]() {
                 FRecalledMemory Memory;
                 Memory.Text =
                     Object->GetStringField(Data.RecalledMemory.Text);
                 Memory.Type =
                     Object->GetStringField(Data.RecalledMemory.Type);
                 Memory.Importance = static_cast<float>(Importance);
                 Memory.Similarity = static_cast<float>(Similarity);
                 return func::just(Memory);
               }();
}

/** User Story: As an FP collection pipeline, I need recalled-memory values lifted into Maybe so array traversal remains all-or-nothing. @fn inline func::Maybe<FRecalledMemory> DecodeRecalledMemoryValue(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FRecalledMemory>
DecodeRecalledMemoryValue(const TSharedPtr<FJsonValue> &Value) {
  return !Value.IsValid() || Value->Type != EJson::Object
             ? func::nothing<FRecalledMemory>()
             : DecodeRecalledMemoryObject(Value->AsObject());
}

/** User Story: As a api serialization memory consumer, I need to invoke memory item to object through a stable signature so the api serialization memory workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> MemoryItemToObject(const FMemoryItem &Memory) */
inline TSharedRef<FJsonObject> MemoryItemToObject(const FMemoryItem &Memory) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  const TArray<TSharedPtr<FJsonValue>> Embedding =
      func::map_array<float, TSharedPtr<FJsonValue>>(
          Memory.Embedding, [](float Value) {
            return MakeShared<FJsonValueNumber>(Value);
          });
  return (detail::SetIfNonEmpty(Object, Data.MemoryItem.Id, Memory.Id),
          Object->SetStringField(Data.MemoryItem.Text, Memory.Text),
          Object->SetStringField(Data.MemoryItem.Type, Memory.Type),
          Object->SetArrayField(Data.MemoryItem.Embedding, Embedding),
          Object->SetNumberField(Data.MemoryItem.Importance,
                                 Memory.Importance),
          Object->SetNumberField(Data.MemoryItem.Similarity,
                                 Memory.Similarity),
          Object->SetNumberField(Data.MemoryItem.Timestamp,
                                 static_cast<double>(Memory.Timestamp)),
          Object);
}

/** User Story: As a Soul-memory consumer, I need exported memory items decoded from the complete canonical shape so malformed entries reject their containing Soul. @fn inline func::Maybe<FMemoryItem> DecodeMemoryItemObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FMemoryItem>
DecodeMemoryItemObject(const TSharedPtr<FJsonObject> &Object) {
  const MemoryConfiguration::FMemoryContractData &Data =
      MemoryConfiguration::memoryContractData();
  double Importance = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  double Similarity = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  double Timestamp = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
  const TArray<TSharedPtr<FJsonValue>> *Embedding = nullptr;
  const bool bValid =
      Object.IsValid() &&
      HasOptionalFieldType(Object, Data.MemoryItem.Id, EJson::String) &&
      Object->HasTypedField<EJson::String>(Data.MemoryItem.Text) &&
      Object->HasTypedField<EJson::String>(Data.MemoryItem.Type) &&
      HasOptionalFieldType(Object, Data.MemoryItem.Embedding, EJson::Array) &&
      Object->TryGetNumberField(Data.MemoryItem.Importance, Importance) &&
      Object->TryGetNumberField(Data.MemoryItem.Similarity, Similarity) &&
      Object->TryGetNumberField(Data.MemoryItem.Timestamp, Timestamp) &&
      FMath::IsFinite(Importance) && FMath::IsFinite(Similarity) &&
      FMath::IsFinite(Timestamp);
  const bool bHasEmbedding =
      bValid && HasNonNullField(Object, Data.MemoryItem.Embedding);
  const bool bEmbeddingLoaded =
      !bHasEmbedding ||
      Object->TryGetArrayField(Data.MemoryItem.Embedding, Embedding);
  const func::Maybe<TArray<float>> DecodedEmbedding =
      bValid && bEmbeddingLoaded && Embedding
          ? func::traverse_maybe_array<TSharedPtr<FJsonValue>, float>(
                *Embedding, [](const TSharedPtr<FJsonValue> &Value) {
                  return Value.IsValid() && Value->Type == EJson::Number &&
                                 FMath::IsFinite(Value->AsNumber())
                             ? func::just(static_cast<float>(Value->AsNumber()))
                             : func::nothing<float>();
                })
          : func::just(TArray<float>());
  return !bValid || !bEmbeddingLoaded || func::is_nothing(DecodedEmbedding)
             ? func::nothing<FMemoryItem>()
             : [&]() {
                 FMemoryItem Memory;
                 Memory.Id =
                     OptionalStringFromField(Object, Data.MemoryItem.Id);
                 Memory.Text = Object->GetStringField(Data.MemoryItem.Text);
                 Memory.Type = Object->GetStringField(Data.MemoryItem.Type);
                 Memory.Embedding = DecodedEmbedding.value;
                 Memory.Importance = static_cast<float>(Importance);
                 Memory.Similarity = static_cast<float>(Similarity);
                 Memory.Timestamp = static_cast<int64>(Timestamp);
                 return func::just(Memory);
               }();
}

/** User Story: As an FP Soul decoder, I need memory-item values lifted into Maybe so no malformed export entry can be silently dropped. @fn inline func::Maybe<FMemoryItem> DecodeMemoryItemValue(const TSharedPtr<FJsonValue> &Value) */
inline func::Maybe<FMemoryItem>
DecodeMemoryItemValue(const TSharedPtr<FJsonValue> &Value) {
  return !Value.IsValid() || Value->Type != EJson::Object
             ? func::nothing<FMemoryItem>()
             : DecodeMemoryItemObject(Value->AsObject());
}

} // namespace JsonInterop
