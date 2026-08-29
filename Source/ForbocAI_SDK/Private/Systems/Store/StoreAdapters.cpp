#include "Systems/Store/StoreAdapters.h"

namespace StoreAdapters {

/**
 * Returns the SDK module's canonical runtime store.
 * User Story: As an Unreal consumer module, I need Blueprint, CLI, and native
 * integrations to dispatch through one SDK-owned root across DLL boundaries.
 * @fn rtk::EnhancedStore<FRuntimeState> &RootStore()
 */
rtk::EnhancedStore<FRuntimeState> &RootStore() {
  static rtk::EnhancedStore<FRuntimeState> Root = createRuntimeStore();
  return Root;
}

} // namespace StoreAdapters
