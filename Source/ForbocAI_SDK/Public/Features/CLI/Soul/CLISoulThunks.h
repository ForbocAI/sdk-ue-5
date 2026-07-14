#pragma once

// Parent-domain qualification disambiguates CLI orchestration from Soul thunks.

#include "Core/rtk.hpp"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Soul/SoulThunks.h"

struct FRuntimeState;

namespace Ops {

template <typename RuntimeState = FRuntimeState>
inline FSoulExportResult exportSoul(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &NpcId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::remoteExportSoulThunk(NpcId)));
}

template <typename RuntimeState = FRuntimeState>
inline FSoul importSoul(rtk::EnhancedStore<RuntimeState> &Store,
                        const FString &TxId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::importSoulFromArweaveThunk(TxId)));
}

template <typename RuntimeState = FRuntimeState>
inline TArray<FSoulListItem>
listSouls(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = 50) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getSoulListThunk(Limit)));
}

template <typename RuntimeState = FRuntimeState>
inline FSoulVerifyResult verifySoul(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &TxId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::verifySoulThunk(TxId)));
}

} // namespace Ops
