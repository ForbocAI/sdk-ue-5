#include "Systems/Vector/VectorAdapters.h"
#include "Systems/Memory/Vector/MemoryVectorAdapters.h"

namespace VectorAdapters {

/** User Story: As a features vector consumer, I need to invoke embed vector adapter through a stable signature so the features vector workflow remains explicit and composable. @fn TArray<float> embedVectorAdapter(const FString &Text) */
TArray<float> embedVectorAdapter(const FString &Text) {
  return MemoryVectorAdapters::embed(Text);
}

} // namespace VectorAdapters
