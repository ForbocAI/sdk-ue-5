#include "Features/Vector/VectorAdapters.h"
#include "Features/Memory/Vector/MemoryVectorAdapters.h"

namespace VectorAdapters {

TArray<float> embedVectorAdapter(const FString &Text) {
  return MemoryVectorAdapters::embed(Text);
}

} // namespace VectorAdapters
