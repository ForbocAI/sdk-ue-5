#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"
#include "Features/Memory/MemoryTypes.h"

namespace Native::Sqlite {

/** User Story: As local memory persistence, I need upsert to return the exact stored record or an explicit error so Redux cannot acknowledge failed writes. @fn FORBOCAI_SDK_API func::Either<FString, FMemoryItem> upsert(DB Database, const FMemoryItem &Item) */
FORBOCAI_SDK_API func::Either<FString, FMemoryItem>
upsert(DB Database, const FMemoryItem &Item);

} // namespace Native::Sqlite
