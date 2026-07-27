#pragma once

#if WITH_FORBOC_SQLITE_VEC
#include <memory>

struct sqlite3_stmt;

namespace Native::Sqlite::StatementTypes {

using FStatement = std::shared_ptr<sqlite3_stmt>;

} // namespace Native::Sqlite::StatementTypes
#endif
