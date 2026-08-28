#pragma once

#include "Core/rtk.hpp"
#include "Components/Memory/Local/LocalTypes.h"
#include "Components/Memory/MemoryTypes.h"

struct FRuntimeState;

namespace rtk {

/** User Story: As native memory setup, I need an NPC-scoped database verified before use so callers do not manage SQLite handles. @fn FORBOCAI_SDK_API ThunkAction<MemoryLocalTypes::FMemoryDatabasePaths, FRuntimeState> initNodeMemoryThunk(const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<MemoryLocalTypes::FMemoryDatabasePaths,
                             FRuntimeState>
initNodeMemoryThunk(const FString &DatabaseName);

/** User Story: As native memory persistence, I need new records created and stored through one RTK effect boundary. @fn FORBOCAI_SDK_API ThunkAction<FMemoryItem, FRuntimeState> storeNodeMemoryThunk(const FString &Text, const FString &Type, float Importance, const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<FMemoryItem, FRuntimeState>
storeNodeMemoryThunk(const FString &Text, const FString &Type,
                     float Importance, const FString &DatabaseName);

/** User Story: As native memory import, I need existing records upserted without changing identity or chronology. @fn FORBOCAI_SDK_API ThunkAction<FMemoryItem, FRuntimeState> upsertNodeMemoryThunk(const FMemoryItem &Item, const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<FMemoryItem, FRuntimeState>
upsertNodeMemoryThunk(const FMemoryItem &Item, const FString &DatabaseName);

/** User Story: As native memory recall, I need semantic search scoped to one NPC database and validated before Redux receives results. @fn FORBOCAI_SDK_API ThunkAction<TArray<FMemoryItem>, FRuntimeState> recallNodeMemoryThunk(const FString &Query, int32 Limit, float Threshold, const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<TArray<FMemoryItem>, FRuntimeState>
recallNodeMemoryThunk(const FString &Query, int32 Limit, float Threshold,
                      const FString &DatabaseName);

/** User Story: As native memory inspection, I need persisted pages read directly from the NPC database so diagnostics do not depend on cache history. @fn FORBOCAI_SDK_API ThunkAction<TArray<FMemoryItem>, FRuntimeState> listNodeMemoryThunk(int32 Limit, int32 Offset, const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<TArray<FMemoryItem>, FRuntimeState>
listNodeMemoryThunk(int32 Limit, int32 Offset, const FString &DatabaseName);

/** User Story: As native memory lifecycle, I need all database artifacts removed before Redux clears its entities. @fn FORBOCAI_SDK_API ThunkAction<FEmptyPayload, FRuntimeState> clearNodeMemoryThunk(const FString &DatabaseName) */
FORBOCAI_SDK_API ThunkAction<FEmptyPayload, FRuntimeState>
clearNodeMemoryThunk(const FString &DatabaseName);

} // namespace rtk
