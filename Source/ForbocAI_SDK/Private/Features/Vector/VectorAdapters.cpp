#include "Features/Vector/VectorAdapters.h"
#include "Core/SdkVectorizer.h"

namespace VectorAdapters {

TArray<float> embedVectorAdapter(const FString &Text) {
  return ForbocAI::SDK::Vectorizer::Embed(Text);
}

} // namespace VectorAdapters
