#pragma once

#include "Store.h"

namespace StoreAdapters {

/**
 * Returns the package-owned root store used by SDK integration boundaries.
 * User Story: As an SDK host boundary, I need one explicit root-store adapter
 * so CLI and Unreal integrations share state without creating competing stores.
 * @fn inline rtk::EnhancedStore<FRuntimeState> &RootStore()
 */
inline rtk::EnhancedStore<FRuntimeState> &RootStore() { return store(); }

} // namespace StoreAdapters
