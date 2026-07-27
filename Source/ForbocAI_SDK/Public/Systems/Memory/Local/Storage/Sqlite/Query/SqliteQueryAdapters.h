#pragma once

#include "Core/fp.hpp"
#include "Components/Memory/Local/Storage/Sqlite/SqliteTypes.h"
#include "Components/Memory/MemoryTypes.h"

namespace Native::Sqlite {

/** User Story: As local memory recall, I need vector search to return rows or an explicit native failure so failed queries cannot masquerade as no matches. @fn FORBOCAI_SDK_API func::Either<FString, TArray<FMemoryItem>> search(DB Database, const TArray<float> &Vector, int32 TopK) */
FORBOCAI_SDK_API func::Either<FString, TArray<FMemoryItem>>
search(DB Database, const TArray<float> &Vector, int32 TopK);

/** User Story: As local memory inspection, I need persisted records paged through SQLite so export and diagnostics read storage rather than Redux history. @fn FORBOCAI_SDK_API func::Either<FString, TArray<FMemoryItem>> list(DB Database, int32 Limit, int32 Offset) */
FORBOCAI_SDK_API func::Either<FString, TArray<FMemoryItem>>
list(DB Database, int32 Limit, int32 Offset);

} // namespace Native::Sqlite
