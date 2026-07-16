#include "Features/Memory/Local/Storage/Sqlite/Statement/StatementAdapters.h"

#include "Features/Memory/Configuration/ConfigurationAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite3.h"

namespace Native::Sqlite::StatementAdapters {

/** User Story: As SQLite effects, I need prepared statements finalized automatically so every success and failure path releases native resources. @fn func::Either<FString, StatementTypes::FStatement> prepareStatementAdapter(sqlite3 *Database, const FString &Sql) */
func::Either<FString, StatementTypes::FStatement>
prepareStatementAdapter(sqlite3 *Database, const FString &Sql) {
  sqlite3_stmt *RawStatement = nullptr;
  const FTCHARToUTF8 SqlUtf8(*Sql);
  const bool bPrepared = Database &&
      sqlite3_prepare_v2(
          Database, SqlUtf8.Get(),
          MemoryConfiguration::memoryData().Iteration.SqliteVariableLength,
          &RawStatement, nullptr) == SQLITE_OK;
  const StatementTypes::FStatement Statement(
      RawStatement, [](sqlite3_stmt *Value) {
        Value ? (sqlite3_finalize(Value), void()) : void();
      });
  return bPrepared
             ? func::make_right<FString, StatementTypes::FStatement>(
                   Statement)
             : func::make_left<FString, StatementTypes::FStatement>(
                   MemoryConfiguration::memoryData()
                       .Errors.SqlitePrepareFailed);
}

} // namespace Native::Sqlite::StatementAdapters
#endif
