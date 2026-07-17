#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"

namespace Native::Sqlite::MigrationAdapters {

/** User Story: As a persistent-memory consumer, I need SQLite opened under exactly one current vector contract so legacy records are migrated and newer records fail closed. @fn func::Either<FString, bool> ensureMemoryContractAdapter(DB Database) */
func::Either<FString, bool> ensureMemoryContractAdapter(DB Database);

} // namespace Native::Sqlite::MigrationAdapters
