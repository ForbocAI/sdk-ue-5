#pragma once

#include "Components/API/Serialization/Memory/Configuration/SerializationMemoryConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace JsonInterop::MemoryConfiguration {

/** User Story: As a Memory codec consumer, I need one immutable authored field contract so request and response serialization cannot drift independently. @fn inline const FMemoryContractData &memoryContractData() */
inline const FMemoryContractData &memoryContractData() {
  static const FMemoryContractData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/api/memory.json"));
    const TSharedRef<FJsonObject> StoreInstruction =
        DataAdapters::ReadObjectField(Source, TEXT("storeInstruction"));
    const TSharedRef<FJsonObject> RecalledMemory =
        DataAdapters::ReadObjectField(Source, TEXT("recalledMemory"));
    const TSharedRef<FJsonObject> MemoryItem =
        DataAdapters::ReadObjectField(Source, TEXT("memoryItem"));
    return FMemoryContractData{
        {DataAdapters::ReadStringField(StoreInstruction, TEXT("text")),
         DataAdapters::ReadStringField(StoreInstruction, TEXT("type")),
         DataAdapters::ReadStringField(StoreInstruction, TEXT("importance"))},
        {DataAdapters::ReadStringField(RecalledMemory, TEXT("text")),
         DataAdapters::ReadStringField(RecalledMemory, TEXT("type")),
         DataAdapters::ReadStringField(RecalledMemory, TEXT("importance")),
         DataAdapters::ReadStringField(RecalledMemory, TEXT("similarity"))},
        {DataAdapters::ReadStringField(MemoryItem, TEXT("id")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("text")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("embedding")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("type")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("importance")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("similarity")),
         DataAdapters::ReadStringField(MemoryItem, TEXT("timestamp"))}};
  }();
  return Data;
}

} // namespace JsonInterop::MemoryConfiguration
