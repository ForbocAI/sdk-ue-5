#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"

namespace DependenciesProcessAdapters {

/** User Story: As a features dependencies process consumer, I need to invoke find subdir with prefix through a stable signature so the features dependencies process workflow remains explicit and composable. @fn FORBOCAI_SDK_API FString findSubdirWithPrefix(const FString &Directory, const FString &Prefix) */
FORBOCAI_SDK_API FString findSubdirWithPrefix(const FString &Directory,
                                               const FString &Prefix);

/**
 * User Story: As a features dependencies process consumer, I need to invoke run process through a stable signature so the features dependencies process workflow remains explicit and composable.
 * @fn FORBOCAI_SDK_API int32 runProcess(const FString &Executable, const FString &Arguments, const FString &WorkingDirectory = TEXT(""), float TimeoutSeconds = FORBOCAI_SDK_AUTHORED_NUMBERV4340F6558590)
 */
FORBOCAI_SDK_API int32 runProcess(const FString &Executable,
                                  const FString &Arguments,
                                  const FString &WorkingDirectory = TEXT(""),
                                  float TimeoutSeconds = FORBOCAI_SDK_AUTHORED_NUMBERV4340F6558590);

/** User Story: As a features dependencies process consumer, I need to invoke extract zip through a stable signature so the features dependencies process workflow remains explicit and composable. @fn FORBOCAI_SDK_API bool extractZip(const FString &ArchivePath, const FString &DestinationDirectory) */
FORBOCAI_SDK_API bool extractZip(const FString &ArchivePath,
                                 const FString &DestinationDirectory);

} // namespace DependenciesProcessAdapters
