#pragma once

#include "CoreMinimal.h"

namespace VectorAdapters {

/** User Story: As a features vector consumer, I need to invoke embed vector adapter through a stable signature so the features vector workflow remains explicit and composable. @fn FORBOCAI_SDK_API TArray<float> embedVectorAdapter(const FString &Text) */
FORBOCAI_SDK_API TArray<float>
embedVectorAdapter(const FString &Text);

} // namespace VectorAdapters
