#pragma once

#include "Features/Dependencies/DependenciesTypes.h"

namespace DependenciesAdapters {

FORBOCAI_SDK_API FNativeDependenciesReport checkNativeDependenciesAdapter();

FORBOCAI_SDK_API FString clearVectorArtifactsAdapter();

FORBOCAI_SDK_API FDependenciesResult
setupNativeDependenciesAdapter(const FDependenciesOptions &Options);

} // namespace DependenciesAdapters
