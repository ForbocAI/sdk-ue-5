#pragma once

#include "Core/JsonInterop.h"
#include "Features/Contracts/ContractsTypes.h"

namespace JsonInterop {

inline TSharedRef<FJsonObject>
MemoryStoreInstructionToObject(const FMemoryStoreInstruction &Instruction) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(TEXT("text"), Instruction.Text),
          Object->SetStringField(TEXT("type"), Instruction.Type),
          Object->SetNumberField(TEXT("importance"), Instruction.Importance),
          Object);
}

inline FMemoryStoreInstruction
MemoryStoreInstructionFromObject(const TSharedPtr<FJsonObject> &Object) {
  FMemoryStoreInstruction Instruction;
  return !Object.IsValid()
             ? Instruction
             : (Instruction.Text = Object->GetStringField(TEXT("text")),
                Instruction.Type = Object->GetStringField(TEXT("type")),
                Instruction.Importance = detail::TryGetNumberAs<float>(
                    Object, TEXT("importance"), Instruction.Importance),
                Instruction);
}

inline TSharedRef<FJsonObject>
RecalledMemoryToObject(const FRecalledMemory &Memory) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Object->SetStringField(TEXT("text"), Memory.Text),
          Object->SetStringField(TEXT("type"), Memory.Type),
          Object->SetNumberField(TEXT("importance"), Memory.Importance),
          Object->SetNumberField(TEXT("similarity"), Memory.Similarity),
          Object);
}

inline FRecalledMemory
RecalledMemoryFromObject(const TSharedPtr<FJsonObject> &Object) {
  FRecalledMemory Memory;
  return !Object.IsValid()
             ? Memory
             : (Memory.Text = Object->GetStringField(TEXT("text")),
                Memory.Type = Object->GetStringField(TEXT("type")),
                Memory.Importance = detail::TryGetNumberAs<float>(
                    Object, TEXT("importance"), Memory.Importance),
                Memory.Similarity = detail::TryGetNumberAs<float>(
                    Object, TEXT("similarity"), Memory.Similarity),
                Memory);
}

inline TSharedRef<FJsonObject> MemoryItemToObject(const FMemoryItem &Memory) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (detail::SetIfNonEmpty(Object, TEXT("id"), Memory.Id),
          Object->SetStringField(TEXT("text"), Memory.Text),
          Object->SetStringField(TEXT("type"), Memory.Type),
          Object->SetNumberField(TEXT("importance"), Memory.Importance),
          Object->SetNumberField(TEXT("similarity"), Memory.Similarity),
          Object->SetNumberField(TEXT("timestamp"),
                                 static_cast<double>(Memory.Timestamp)),
          Object);
}

inline FMemoryItem MemoryItemFromObject(const TSharedPtr<FJsonObject> &Object) {
  FMemoryItem Memory;
  return !Object.IsValid()
             ? Memory
             : (Memory.Id = Object->GetStringField(TEXT("id")),
                Memory.Text = Object->GetStringField(TEXT("text")),
                Memory.Type = Object->GetStringField(TEXT("type")),
                Memory.Importance = detail::TryGetNumberAs<float>(
                    Object, TEXT("importance"), Memory.Importance),
                Memory.Similarity = detail::TryGetNumberAs<float>(
                    Object, TEXT("similarity"), Memory.Similarity),
                Memory.Timestamp = detail::TryGetNumberAs<int64>(
                    Object, TEXT("timestamp"), Memory.Timestamp),
                Memory);
}

} // namespace JsonInterop
