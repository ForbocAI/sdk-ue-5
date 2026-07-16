#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/MemoryTypes.h"

#if WITH_FORBOC_SQLITE_VEC
struct sqlite3_stmt;
#endif

namespace Native::Sqlite::CodecAdapters {

/** User Story: As SQLite vector storage, I need vectors encoded through the authored wire format so schema changes stay outside source. @fn FString buildJsonVector(const TArray<float> &Vector) */
FString buildJsonVector(const TArray<float> &Vector);

/** User Story: As SQLite vector storage, I need persisted records normalized without inventing identity so imports remain lossless. @fn FMemoryItem prepareStoredItem(const FMemoryItem &Item) */
FMemoryItem prepareStoredItem(const FMemoryItem &Item);

#if WITH_FORBOC_SQLITE_VEC
/** User Story: As SQLite recall, I need row decoding to distinguish completion from native errors so failed queries cannot look empty. @fn func::Either<FString, TArray<FMemoryItem>> collectSearchRows(sqlite3_stmt *Statement) */
func::Either<FString, TArray<FMemoryItem>>
collectSearchRows(sqlite3_stmt *Statement);
#endif

} // namespace Native::Sqlite::CodecAdapters
