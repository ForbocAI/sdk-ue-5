#pragma once

#include "Core/rtk.hpp"
#include "Components/Soul/SoulTypes.h"

struct FRuntimeState;

namespace rtk {

/**
 * User Story: As an NPC owner, I need complete portable state exported through
 * one generated RTK lifecycle so every SDK surface observes the same result.
 * @fn FORBOCAI_SDK_API const AsyncThunkConfig<FSoulExportResult, FString, FRuntimeState> & exportSoulThunk()
 */
FORBOCAI_SDK_API const AsyncThunkConfig<FSoulExportResult, FString,
                                       FRuntimeState> &
exportSoulThunk();

/**
 * User Story: As an SDK consumer, I need a cataloged Soul authorized,
 * retrieved, authenticated, and decrypted through one generated RTK lifecycle.
 * @fn FORBOCAI_SDK_API const AsyncThunkConfig<FSoul, FString, FRuntimeState> & importSoulThunk()
 */
FORBOCAI_SDK_API const AsyncThunkConfig<FSoul, FString, FRuntimeState> &
importSoulThunk();

/**
 * User Story: As an SDK consumer, I need the package-owned Soul catalog listed
 * through one generated RTK lifecycle without API-owned persistence.
 * @fn FORBOCAI_SDK_API const AsyncThunkConfig<TArray<FSoulListItem>, int32, FRuntimeState> & listSoulsThunk()
 */
FORBOCAI_SDK_API const AsyncThunkConfig<TArray<FSoulListItem>, int32,
                                       FRuntimeState> &
listSoulsThunk();

/**
 * User Story: As a Soul owner, I need API authorization and provider bytes
 * verified through one generated RTK lifecycle before validity is reported.
 * @fn FORBOCAI_SDK_API const AsyncThunkConfig<FSoulVerifyResult, FString, FRuntimeState> & verifySoulThunk()
 */
FORBOCAI_SDK_API const AsyncThunkConfig<FSoulVerifyResult, FString,
                                       FRuntimeState> &
verifySoulThunk();

} // namespace rtk
