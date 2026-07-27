#pragma once

#include "Core/fp.hpp"
#include "Components/Memory/Local/Storage/Sqlite/Statement/StatementTypes.h"

#if WITH_FORBOC_SQLITE_VEC
struct sqlite3;

namespace Native::Sqlite::StatementAdapters {

/** User Story: As SQLite effects, I need prepared statements finalized automatically so every success and failure path releases native resources. @fn func::Either<FString, StatementTypes::FStatement> prepareStatementAdapter(sqlite3 *Database, const FString &Sql) */
func::Either<FString, StatementTypes::FStatement>
prepareStatementAdapter(sqlite3 *Database, const FString &Sql);

} // namespace Native::Sqlite::StatementAdapters
#endif
