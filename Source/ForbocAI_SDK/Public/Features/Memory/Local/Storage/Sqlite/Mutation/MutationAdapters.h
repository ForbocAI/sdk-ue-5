#pragma once

#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"
#include "Features/Memory/MemoryTypes.h"

namespace Native::Sqlite {

FORBOCAI_SDK_API bool Upsert(DB Database, const FMemoryItem &Item);

} // namespace Native::Sqlite
