#include "Features/Memory/Local/Storage/Sqlite/Query/SqliteQueryAdapters.h"

#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Statement/StatementAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite3.h"
#endif

namespace Native::Sqlite {

#if WITH_FORBOC_SQLITE_VEC
namespace {

/**
 * User Story: As local memory recall, I need native search inputs validated before statement allocation so invalid requests cannot touch SQLite.
 * @fn func::Either<FString, bool> validateSearchAdapter( sqlite3 *Handle, const TArray<float> &Vector, int32 TopK)
 */
func::Either<FString, bool> validateSearchAdapter(
    sqlite3 *Handle, const TArray<float> &Vector, int32 TopK) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return !Handle
             ? func::make_left<FString, bool>(
                   Data.Errors.LocalNotInitialized)
             : TopK <= Data.Iteration.InitialIndex
                   ? func::make_left<FString, bool>(
                         Data.Errors.SqliteInvalidLimit)
                   : Vector.Num() != Data.Vector.Dimension
                         ? func::make_left<FString, bool>(
                               Data.Errors.InvalidVectorRecord)
                         : func::make_right<FString, bool>(true);
}

/** User Story: As local memory inspection, I need page bounds validated before statement allocation so invalid pages cannot reach SQLite. @fn func::Either<FString, bool> validateListAdapter(sqlite3 *Handle, int32 Limit, int32 Offset) */
func::Either<FString, bool> validateListAdapter(sqlite3 *Handle, int32 Limit,
                                                int32 Offset) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return !Handle
             ? func::make_left<FString, bool>(
                   Data.Errors.LocalNotInitialized)
             : Limit <= Data.Iteration.InitialIndex
                   ? func::make_left<FString, bool>(
                         Data.Errors.SqliteInvalidLimit)
                   : Offset < Data.Iteration.InitialIndex
                         ? func::make_left<FString, bool>(
                               Data.Errors.SqliteInvalidOffset)
                         : func::make_right<FString, bool>(true);
}

} // namespace
#endif

/** User Story: As local memory recall, I need vector search to return rows or an explicit native failure so failed queries cannot masquerade as no matches. @fn func::Either<FString, TArray<FMemoryItem>> search(DB Database, const TArray<float> &Vector, int32 TopK) */
func::Either<FString, TArray<FMemoryItem>>
search(DB Database, const TArray<float> &Vector, int32 TopK) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return func::ebind(
      validateSearchAdapter(Handle, Vector, TopK),
      [Handle, &Vector, TopK, &Data](bool) {
        return func::ematch(
            StatementAdapters::prepareStatementAdapter(
                Handle, Data.Storage.Sqlite.Search),
            [](const FString &Error) {
              return func::make_left<FString, TArray<FMemoryItem>>(Error);
            },
            [&Vector, TopK, &Data](
                const StatementTypes::FStatement &Statement) {
              const FString JsonVector =
                  CodecAdapters::buildJsonVector(Vector);
              const bool bBound =
                  sqlite3_bind_text(
                      Statement.get(),
                      Data.Storage.Sqlite.Bindings.QueryEmbedding,
                      TCHAR_TO_UTF8(*JsonVector),
                      Data.Iteration.SqliteVariableLength,
                      SQLITE_TRANSIENT) == SQLITE_OK &&
                  sqlite3_bind_int(
                      Statement.get(),
                      Data.Storage.Sqlite.Bindings.QueryLimit,
                      TopK) == SQLITE_OK;
              return bBound
                         ? CodecAdapters::collectSearchRows(Statement.get())
                         : func::make_left<FString, TArray<FMemoryItem>>(
                               Data.Errors.SqliteBindFailed);
            });
      });
#else
  (void)Database;
  (void)Vector;
  (void)TopK;
  return func::make_left<FString, TArray<FMemoryItem>>(
      Data.Errors.SqliteUnavailable);
#endif
}

/** User Story: As local memory inspection, I need persisted records paged through SQLite so export and diagnostics read storage rather than Redux history. @fn func::Either<FString, TArray<FMemoryItem>> list(DB Database, int32 Limit, int32 Offset) */
func::Either<FString, TArray<FMemoryItem>> list(DB Database, int32 Limit,
                                                int32 Offset) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return func::ebind(
      validateListAdapter(Handle, Limit, Offset),
      [Handle, Limit, Offset, &Data](bool) {
        return func::ematch(
            StatementAdapters::prepareStatementAdapter(
                Handle, Data.Storage.Sqlite.List),
            [](const FString &Error) {
              return func::make_left<FString, TArray<FMemoryItem>>(Error);
            },
            [Limit, Offset, &Data](
                const StatementTypes::FStatement &Statement) {
              const bool bBound =
                  sqlite3_bind_int(
                      Statement.get(), Data.Storage.Sqlite.Bindings.ListLimit,
                      Limit) == SQLITE_OK &&
                  sqlite3_bind_int(
                      Statement.get(), Data.Storage.Sqlite.Bindings.ListOffset,
                      Offset) == SQLITE_OK;
              return bBound
                         ? CodecAdapters::collectSearchRows(Statement.get())
                         : func::make_left<FString, TArray<FMemoryItem>>(
                               Data.Errors.SqliteBindFailed);
            });
      });
#else
  (void)Database;
  (void)Limit;
  (void)Offset;
  return func::make_left<FString, TArray<FMemoryItem>>(
      Data.Errors.SqliteUnavailable);
#endif
}

} // namespace Native::Sqlite
