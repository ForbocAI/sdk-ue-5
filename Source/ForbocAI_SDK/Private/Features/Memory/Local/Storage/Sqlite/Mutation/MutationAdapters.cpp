#include "Features/Memory/Local/Storage/Sqlite/Mutation/MutationAdapters.h"

#include "Features/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
extern "C" {
#include "sqlite3.h"
}
#endif

namespace Native::Sqlite {

bool Upsert(DB Database, const FMemoryItem &Item) {
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return !Handle || Item.Embedding.IsEmpty()
             ? false
             : [&]() -> bool {
                 const char *Sql =
                     "INSERT OR REPLACE INTO memories "
                     "(id, text, type, importance, timestamp, embedding) "
                     "VALUES (?, ?, ?, ?, ?, ?);";
                 sqlite3_stmt *Statement = nullptr;
                 return sqlite3_prepare_v2(Handle, Sql, -1, &Statement,
                                           nullptr) != SQLITE_OK
                            ? false
                            : [&]() -> bool {
                                const FMemoryItem StoredItem =
                                    CodecAdapters::PrepareStoredItem(Item);
                                const FString JsonVector =
                                    CodecAdapters::BuildJsonVector(
                                        StoredItem.Embedding);
                                sqlite3_bind_text(
                                    Statement, 1,
                                    TCHAR_TO_UTF8(*StoredItem.Id), -1,
                                    SQLITE_TRANSIENT);
                                sqlite3_bind_text(
                                    Statement, 2,
                                    TCHAR_TO_UTF8(*StoredItem.Text), -1,
                                    SQLITE_TRANSIENT);
                                sqlite3_bind_text(
                                    Statement, 3,
                                    TCHAR_TO_UTF8(*StoredItem.Type), -1,
                                    SQLITE_TRANSIENT);
                                sqlite3_bind_double(
                                    Statement, 4,
                                    static_cast<double>(StoredItem.Importance));
                                sqlite3_bind_int64(
                                    Statement, 5,
                                    static_cast<sqlite3_int64>(
                                        StoredItem.Timestamp));
                                sqlite3_bind_text(
                                    Statement, 6, TCHAR_TO_UTF8(*JsonVector),
                                    -1, SQLITE_TRANSIENT);
                                const bool bStored =
                                    sqlite3_step(Statement) == SQLITE_DONE;
                                sqlite3_finalize(Statement);
                                return bStored;
                              }();
               }();
#else
  (void)Database;
  (void)Item;
  return false;
#endif
}

} // namespace Native::Sqlite
