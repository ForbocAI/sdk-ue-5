#include "Systems/Memory/Local/Storage/Sqlite/Mutation/MutationAdapters.h"

#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Statement/StatementAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite3.h"
#endif

namespace Native::Sqlite {

#if WITH_FORBOC_SQLITE_VEC
namespace {

/** User Story: As local memory persistence, I need native records validated before statement allocation so malformed vectors cannot reach SQLite. @fn func::Either<FString, bool> validateUpsertAdapter(sqlite3 *Handle, const FMemoryItem &Item) */
func::Either<FString, bool> validateUpsertAdapter(sqlite3 *Handle,
                                                  const FMemoryItem &Item) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const bool bRecord =
      !Item.Id.IsEmpty() && !Item.Text.IsEmpty() && !Item.Type.IsEmpty() &&
      Item.Embedding.Num() == Data.Vector.Dimension;
  return !Handle
             ? func::make_left<FString, bool>(
                   Data.Errors.LocalNotInitialized)
             : !bRecord
                   ? func::make_left<FString, bool>(
                         Data.Errors.InvalidVectorRecord)
                   : func::make_right<FString, bool>(true);
}

} // namespace
#endif

/** User Story: As local memory persistence, I need upsert to return the exact stored record or an explicit error so Redux cannot acknowledge failed writes. @fn func::Either<FString, FMemoryItem> upsert(DB Database, const FMemoryItem &Item) */
func::Either<FString, FMemoryItem> upsert(DB Database,
                                         const FMemoryItem &Item) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return func::ebind(
      validateUpsertAdapter(Handle, Item),
      [Handle, &Item, &Data](bool) {
        return func::ematch(
            StatementAdapters::prepareStatementAdapter(
                Handle, Data.Storage.Sqlite.Upsert),
            [](const FString &Error) {
              return func::make_left<FString, FMemoryItem>(Error);
            },
            [&Item, &Data](const StatementTypes::FStatement &Statement) {
              const FMemoryItem StoredItem =
                  CodecAdapters::prepareStoredItem(Item);
              const FString JsonVector =
                  CodecAdapters::buildJsonVector(StoredItem.Embedding);
              const bool bBound =
                  sqlite3_bind_text(
                      Statement.get(), Data.Storage.Sqlite.Bindings.Id,
                      TCHAR_TO_UTF8(*StoredItem.Id),
                      Data.Iteration.SqliteVariableLength,
                      SQLITE_TRANSIENT) == SQLITE_OK &&
                  sqlite3_bind_text(
                      Statement.get(), Data.Storage.Sqlite.Bindings.Text,
                      TCHAR_TO_UTF8(*StoredItem.Text),
                      Data.Iteration.SqliteVariableLength,
                      SQLITE_TRANSIENT) == SQLITE_OK &&
                  sqlite3_bind_text(
                      Statement.get(), Data.Storage.Sqlite.Bindings.Type,
                      TCHAR_TO_UTF8(*StoredItem.Type),
                      Data.Iteration.SqliteVariableLength,
                      SQLITE_TRANSIENT) == SQLITE_OK &&
                  sqlite3_bind_double(
                      Statement.get(),
                      Data.Storage.Sqlite.Bindings.Importance,
                      static_cast<double>(StoredItem.Importance)) ==
                      SQLITE_OK &&
                  sqlite3_bind_int64(
                      Statement.get(), Data.Storage.Sqlite.Bindings.Timestamp,
                      static_cast<sqlite3_int64>(StoredItem.Timestamp)) ==
                      SQLITE_OK &&
                  sqlite3_bind_text(
                      Statement.get(), Data.Storage.Sqlite.Bindings.Embedding,
                      TCHAR_TO_UTF8(*JsonVector),
                      Data.Iteration.SqliteVariableLength,
                      SQLITE_TRANSIENT) == SQLITE_OK;
              return !bBound
                         ? func::make_left<FString, FMemoryItem>(
                               Data.Errors.SqliteBindFailed)
                         : sqlite3_step(Statement.get()) == SQLITE_DONE
                               ? func::make_right<FString, FMemoryItem>(
                                     StoredItem)
                               : func::make_left<FString, FMemoryItem>(
                                     Data.Errors.SqliteStepFailed);
            });
      });
#else
  (void)Database;
  (void)Item;
  return func::make_left<FString, FMemoryItem>(Data.Errors.SqliteUnavailable);
#endif
}

} // namespace Native::Sqlite
