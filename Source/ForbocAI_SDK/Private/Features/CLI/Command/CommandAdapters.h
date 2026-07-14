#pragma once

#include "CoreMinimal.h"

namespace CommandAdapters {

TArray<FString> BuildCommandArgs(const FString &Command,
                                 const FString &Params);

} // namespace CommandAdapters
