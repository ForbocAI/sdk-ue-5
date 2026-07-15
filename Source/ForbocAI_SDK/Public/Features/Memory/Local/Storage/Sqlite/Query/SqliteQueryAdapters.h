#pragma once

#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"
#include "Features/Memory/MemoryTypes.h"

namespace Native::Sqlite {

FORBOCAI_SDK_API TArray<FMemoryItem>
SearchRows(DB Database, const TArray<float> &Vector, int32 TopK = 5);

FORBOCAI_SDK_API TArray<FMemoryItem>
Search(DB Database, const TArray<float> &Vector, int32 TopK = 5);

} // namespace Native::Sqlite
