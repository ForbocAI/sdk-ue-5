#pragma once

#include "Store.h"

namespace StoreAdapters {

/**
 * Returns the package-owned root store used by SDK integration boundaries.
 * User Story: As an SDK host boundary, I need one explicit root-store adapter
 * so CLI and Unreal integrations share state without creating competing stores.
 * @fn FORBOCAI_SDK_API rtk::EnhancedStore<FRuntimeState> &RootStore()
 */
FORBOCAI_SDK_API rtk::EnhancedStore<FRuntimeState> &RootStore();

} // namespace StoreAdapters
