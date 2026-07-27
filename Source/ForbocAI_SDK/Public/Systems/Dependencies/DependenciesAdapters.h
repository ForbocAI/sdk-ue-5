#pragma once

#include "Components/Dependencies/DependenciesTypes.h"

namespace DependenciesAdapters {

/** User Story: As a features dependencies consumer, I need to invoke check native dependencies adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FORBOCAI_SDK_API FNativeDependenciesReport checkNativeDependenciesAdapter() */
FORBOCAI_SDK_API FNativeDependenciesReport checkNativeDependenciesAdapter();

/** User Story: As a features dependencies consumer, I need to invoke clear vector artifacts adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FORBOCAI_SDK_API FString clearVectorArtifactsAdapter() */
FORBOCAI_SDK_API FString clearVectorArtifactsAdapter();

/** User Story: As a features dependencies consumer, I need to invoke setup native dependencies adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FORBOCAI_SDK_API FDependenciesResult setupNativeDependenciesAdapter(const FDependenciesOptions &Options) */
FORBOCAI_SDK_API FDependenciesResult
setupNativeDependenciesAdapter(const FDependenciesOptions &Options);

} // namespace DependenciesAdapters
