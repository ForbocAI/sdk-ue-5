#include "Features/Memory/Local/Storage/Sqlite/Migration/MigrationAdapters.h"

#include "Features/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Features/Memory/Local/LocalAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Mutation/MutationAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Statement/StatementAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite3.h"
#endif

namespace Native::Sqlite::MigrationAdapters {
namespace {

#if WITH_FORBOC_SQLITE_VEC
/** User Story: As a SQLite migration maintainer, I need every authored control statement return native failure details so a migration cannot report false success. @fn func::Either<FString, bool> executeSqlAdapter(sqlite3 *Handle, const FString &Sql, const FString &ErrorPrefix) */
func::Either<FString, bool> executeSqlAdapter(sqlite3 *Handle,
                                              const FString &Sql,
                                              const FString &ErrorPrefix) {
  char *NativeError = nullptr;
  const FTCHARToUTF8 SqlUtf8(*Sql);
  const int32 Result =
      sqlite3_exec(Handle, SqlUtf8.Get(), nullptr, nullptr, &NativeError);
  const FString Error =
      Result == SQLITE_OK
          ? MemoryConfiguration::memoryData().Text.Empty
          : ErrorPrefix +
                (NativeError ? UTF8_TO_TCHAR(NativeError)
                             : UTF8_TO_TCHAR(sqlite3_errmsg(Handle)));
  NativeError ? sqlite3_free(NativeError) : void();
  return Result == SQLITE_OK
             ? func::make_right<FString, bool>(true)
             : func::make_left<FString, bool>(Error);
}

/** User Story: As a SQLite migration maintainer, I need scalar metadata queries decoded through one checked statement boundary. @fn func::Either<FString, int32> readScalarAdapter(sqlite3 *Handle, const FString &Sql) */
func::Either<FString, int32> readScalarAdapter(sqlite3 *Handle,
                                               const FString &Sql) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return func::ematch(
      StatementAdapters::prepareStatementAdapter(Handle, Sql),
      [](const FString &Error) {
        return func::make_left<FString, int32>(Error);
      },
      [&Data](const StatementTypes::FStatement &Statement) {
        return sqlite3_step(Statement.get()) == SQLITE_ROW
                   ? func::make_right<FString, int32>(sqlite3_column_int(
                         Statement.get(), Data.Iteration.InitialIndex))
                   : func::make_left<FString, int32>(
                         Data.Errors.SqliteStepFailed);
      });
}

/** User Story: As a vector-contract migrator, I need every source record read before transaction mutation so failed re-embedding leaves the original table untouched. @fn func::Either<FString, TArray<FMemoryItem>> readAllItemsAdapter(sqlite3 *Handle) */
func::Either<FString, TArray<FMemoryItem>>
readAllItemsAdapter(sqlite3 *Handle) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return func::ematch(
      StatementAdapters::prepareStatementAdapter(
          Handle, Data.Storage.Sqlite.ListAll),
      [](const FString &Error) {
        return func::make_left<FString, TArray<FMemoryItem>>(Error);
      },
      [](const StatementTypes::FStatement &Statement) {
        return CodecAdapters::collectSearchRows(Statement.get());
      });
}

/** User Story: As a vector-contract migrator, I need regenerated records persisted recursively so native writes preserve explicit failure state without loop mutation. @fn func::Either<FString, bool> persistItemsAdapter(DB Database, const TArray<FMemoryItem> &Items, int32 Index) */
func::Either<FString, bool>
persistItemsAdapter(DB Database, const TArray<FMemoryItem> &Items,
                    int32 Index) {
  return Index >= Items.Num()
             ? func::make_right<FString, bool>(true)
             : func::ebind(
                   Native::Sqlite::upsert(Database, Items[Index]),
                   [Database, &Items, Index](const FMemoryItem &) {
                     return persistItemsAdapter(
                         Database, Items,
                         Index + MemoryConfiguration::memoryData()
                                     .Iteration.Step);
                   });
}

/** User Story: As a vector-contract migrator, I need the current contract version written only through the authored PRAGMA template. @fn func::Either<FString, bool> writeCurrentVersionAdapter(sqlite3 *Handle) */
func::Either<FString, bool> writeCurrentVersionAdapter(sqlite3 *Handle) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return executeSqlAdapter(
      Handle,
      FString::Format(*Data.Storage.Sqlite.WriteUserVersion,
                      {Data.Contract.VectorVersion}),
      Data.Errors.MemoryContractInvalid);
}

/** User Story: As a failed migration consumer, I need the transaction rolled back before the original error is returned so legacy memories remain recoverable. @fn func::Either<FString, bool> rollbackMigrationAdapter(sqlite3 *Handle, const FString &Failure) */
func::Either<FString, bool> rollbackMigrationAdapter(sqlite3 *Handle,
                                                     const FString &Failure) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const func::Either<FString, bool> RolledBack = executeSqlAdapter(
      Handle, Data.Storage.Sqlite.RollbackTransaction,
      Data.Errors.MemoryContractMigrationFailed);
  return func::make_left<FString, bool>(
      Data.Errors.MemoryContractMigrationFailed + Failure +
      (RolledBack.isLeft ? RolledBack.left : Data.Text.Empty));
}

/** User Story: As a new memory database consumer, I need the current table and vector contract created together so an unversioned database is never published. @fn func::Either<FString, bool> initializeCurrentContractAdapter(sqlite3 *Handle) */
func::Either<FString, bool>
initializeCurrentContractAdapter(sqlite3 *Handle) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return func::ebind(
      executeSqlAdapter(Handle, Data.Storage.Sqlite.CreateVectorTable,
                        Data.Errors.SqliteSchemaFailed),
      [Handle](bool) { return writeCurrentVersionAdapter(Handle); });
}

/** User Story: As an existing memory database consumer, I need all legacy records re-embedded inside one SQLite transaction so old and current vectors cannot coexist. @fn func::Either<FString, bool> migrateLegacyContractAdapter(DB Database, sqlite3 *Handle) */
func::Either<FString, bool> migrateLegacyContractAdapter(DB Database,
                                                         sqlite3 *Handle) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return func::ematch(
      readAllItemsAdapter(Handle),
      [](const FString &Error) {
        return func::make_left<FString, bool>(Error);
      },
      [Database, Handle, &Data](const TArray<FMemoryItem> &SourceItems) {
        const TArray<FMemoryItem> CurrentItems =
            func::map_array<FMemoryItem, FMemoryItem>(
                SourceItems,
                [](const FMemoryItem &Item) {
                  return MemoryLocalAdapters::
                      createMemoryVectorRecordAdapter(Item);
                });
        return func::ebind(
            executeSqlAdapter(Handle, Data.Storage.Sqlite.BeginTransaction,
                              Data.Errors.MemoryContractMigrationFailed),
            [Database, Handle, &Data, &CurrentItems](bool) {
              const func::Either<FString, bool> Attempt = func::ebind(
                  executeSqlAdapter(Handle,
                                    Data.Storage.Sqlite.DropVectorTable,
                                    Data.Errors.SqliteSchemaFailed),
                  [Database, Handle, &Data, &CurrentItems](bool) {
                    return func::ebind(
                        executeSqlAdapter(
                            Handle, Data.Storage.Sqlite.CreateVectorTable,
                            Data.Errors.SqliteSchemaFailed),
                        [Database, Handle, &Data, &CurrentItems](bool) {
                          return func::ebind(
                              persistItemsAdapter(
                                  Database, CurrentItems,
                                  Data.Iteration.InitialIndex),
                              [Handle, &Data](bool) {
                                return func::ebind(
                                    writeCurrentVersionAdapter(Handle),
                                    [Handle, &Data](bool) {
                                      return executeSqlAdapter(
                                          Handle,
                                          Data.Storage.Sqlite
                                              .CommitTransaction,
                                          Data.Errors
                                              .MemoryContractMigrationFailed);
                                    });
                              });
                        });
                  });
              return func::ematch(
                  Attempt,
                  [Handle](const FString &Error) {
                    return rollbackMigrationAdapter(Handle, Error);
                  },
                  [](bool) { return func::make_right<FString, bool>(true); });
            });
      });
}
#endif

} // namespace

/** User Story: As a persistent-memory consumer, I need SQLite opened under exactly one current vector contract so legacy records are migrated and newer records fail closed. @fn func::Either<FString, bool> ensureMemoryContractAdapter(DB Database) */
func::Either<FString, bool> ensureMemoryContractAdapter(DB Database) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return !Handle
             ? func::make_left<FString, bool>(
                   Data.Errors.LocalNotInitialized)
             : func::ebind(
                   readScalarAdapter(Handle,
                                     Data.Storage.Sqlite.TableExists),
                   [Database, Handle, &Data](int32 TableCount) {
                     return func::ebind(
                         readScalarAdapter(
                             Handle, Data.Storage.Sqlite.ReadUserVersion),
                         [Database, Handle, &Data,
                          TableCount](int32 StoredVersion) {
                           const bool bTableExists =
                               TableCount > Data.Iteration.InitialIndex;
                           return StoredVersion > Data.Contract.VectorVersion
                                      ? func::make_left<FString, bool>(
                                            Data.Errors.MemoryContractNewer)
                                  : !bTableExists
                                      ? initializeCurrentContractAdapter(
                                            Handle)
                                  : StoredVersion ==
                                            Data.Contract.VectorVersion
                                      ? func::make_right<FString, bool>(true)
                                      : migrateLegacyContractAdapter(Database,
                                                                     Handle);
                         });
                   });
#else
  (void)Database;
  return func::make_left<FString, bool>(Data.Errors.SqliteUnavailable);
#endif
}

} // namespace Native::Sqlite::MigrationAdapters
