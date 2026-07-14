#pragma once

#include "CoreMinimal.h"

namespace DependenciesProcessAdapters {

FORBOCAI_SDK_API FString findSubdirWithPrefix(const FString &Directory,
                                               const FString &Prefix);

FORBOCAI_SDK_API int32 runProcess(const FString &Executable,
                                  const FString &Arguments,
                                  const FString &WorkingDirectory = TEXT(""),
                                  float TimeoutSeconds = 300.0f);

FORBOCAI_SDK_API bool extractZip(const FString &ArchivePath,
                                 const FString &DestinationDirectory);

} // namespace DependenciesProcessAdapters
