#pragma once

#include "Features/API/Serialization/Agent/AgentAdapters.h"
#include "Features/API/Serialization/Memory/SerializationMemoryAdapters.h"

namespace JsonInterop {
namespace detail {

inline void ExtractMemoryItemsRecursive(
    const TArray<TSharedPtr<FJsonValue>> &Source, TArray<FMemoryItem> &Output,
    int32 Index) {
  Index < Source.Num()
      ? ((Source[Index].IsValid() && Source[Index]->Type == EJson::Object)
             ? (Output.Add(MemoryItemFromObject(Source[Index]->AsObject())),
                void())
             : void(),
         ExtractMemoryItemsRecursive(Source, Output, Index + 1), void())
      : void();
}

} // namespace detail

inline FSoul SoulFromObject(const TSharedPtr<FJsonObject> &Object) {
  FSoul Soul;
  return !Object.IsValid()
             ? Soul
             : (Soul.Id = OptionalStringFromField(Object, TEXT("id")),
                Soul.Version =
                    OptionalStringFromField(Object, TEXT("version")),
                Soul.Name = OptionalStringFromField(Object, TEXT("name")),
                Soul.Persona =
                    OptionalStringFromField(Object, TEXT("persona")),
                Soul.Signature =
                    OptionalStringFromField(Object, TEXT("signature")),
                Soul.State = StateFromField(Object, TEXT("state")),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *Memories = nullptr;
                  Object->TryGetArrayField(TEXT("memories"), Memories) &&
                          Memories
                      ? (Soul.Memories.Empty(Memories->Num()),
                         detail::ExtractMemoryItemsRecursive(
                             *Memories, Soul.Memories, 0),
                         void())
                      : void();
                }(),
                Soul);
}

inline FImportedNpc
ImportedNpcFromObject(const TSharedPtr<FJsonObject> &Object) {
  FImportedNpc Npc;
  return !Object.IsValid()
             ? Npc
             : (Npc.NpcId =
                    OptionalStringFromField(Object, TEXT("npcId")),
                Npc.Persona =
                    OptionalStringFromField(Object, TEXT("persona")),
                Npc.DataJson = JsonStringFromField(Object, TEXT("data")), Npc);
}

} // namespace JsonInterop
