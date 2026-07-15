#pragma once

#include "Features/Memory/MemoryTypes.h"

#if WITH_FORBOC_SQLITE_VEC
struct sqlite3_stmt;
#endif

namespace Native::Sqlite::CodecAdapters {

FString BuildJsonVector(const TArray<float> &Vector);
FMemoryItem PrepareStoredItem(const FMemoryItem &Item);

#if WITH_FORBOC_SQLITE_VEC
void CollectSearchRows(sqlite3_stmt *Statement,
                       TArray<FMemoryItem> &Results);
#endif

} // namespace Native::Sqlite::CodecAdapters
