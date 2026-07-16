#pragma once

#include "CoreMinimal.h"

namespace JsonInterop::MemoryConfiguration {

struct FMemoryStoreInstructionFields {
  FString Text;
  FString Type;
  FString Importance;
};

struct FRecalledMemoryFields {
  FString Text;
  FString Type;
  FString Importance;
  FString Similarity;
};

struct FMemoryItemFields {
  FString Id;
  FString Text;
  FString Embedding;
  FString Type;
  FString Importance;
  FString Similarity;
  FString Timestamp;
};

struct FMemoryContractData {
  FMemoryStoreInstructionFields StoreInstruction;
  FRecalledMemoryFields RecalledMemory;
  FMemoryItemFields MemoryItem;
};

} // namespace JsonInterop::MemoryConfiguration
