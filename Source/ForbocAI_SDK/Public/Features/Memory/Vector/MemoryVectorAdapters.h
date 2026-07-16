#pragma once

#include "CoreMinimal.h"

namespace MemoryVectorAdapters {

/** User Story: As a features memory vector consumer, I need to invoke embed through a stable signature so the features memory vector workflow remains explicit and composable. @fn FORBOCAI_SDK_API TArray<float> embed(const FString &Text) */
FORBOCAI_SDK_API TArray<float> embed(const FString &Text);

/** User Story: As memory recall, I need one cosine selector shared by all engines so ranking semantics cannot diverge. @fn FORBOCAI_SDK_API float cosineSimilarity(const TArray<float> &A, const TArray<float> &B) */
FORBOCAI_SDK_API float cosineSimilarity(const TArray<float> &A,
                                        const TArray<float> &B);

} // namespace MemoryVectorAdapters
