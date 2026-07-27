#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

// The CLI/Soul folders own the domain; the filename carries only its role.

#include "Core/rtk.hpp"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/CLI/Soul/CLISoulAdapters.h"
#include "Systems/Soul/SoulThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a features cli soul consumer, I need to invoke export soul through a stable signature so the features cli soul workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FSoulExportResult exportSoul(rtk::EnhancedStore<RuntimeState> &Store, const FString &NpcId) */
template <typename RuntimeState = FRuntimeState>
inline FSoulExportResult exportSoul(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &NpcId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::exportSoulThunk()(NpcId)));
}

/**
 * User Story: As a features cli soul consumer, I need to invoke list souls through a stable signature so the features cli soul workflow remains explicit and composable.
 * @fn template <typename RuntimeState = FRuntimeState> inline TArray<FSoulListItem> listSouls(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = FORBOCAI_SDK_AUTHORED_NUMBERV476E954B364F)
 */
template <typename RuntimeState = FRuntimeState>
inline TArray<FSoulListItem>
listSouls(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit = FORBOCAI_SDK_AUTHORED_NUMBERV476E954B364F) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listSoulsThunk()(Limit)));
}

/** User Story: As a features cli soul consumer, I need to invoke verify soul through a stable signature so the features cli soul workflow remains explicit and composable. @fn template <typename RuntimeState = FRuntimeState> inline FSoulVerifyResult verifySoul(rtk::EnhancedStore<RuntimeState> &Store, const FString &TxId) */
template <typename RuntimeState = FRuntimeState>
inline FSoulVerifyResult verifySoul(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &TxId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::verifySoulThunk()(TxId)));
}

} // namespace Ops
