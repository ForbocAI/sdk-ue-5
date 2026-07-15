#include "Features/Memory/Local/Storage/Sqlite/Query/SqliteQueryAdapters.h"

#include "Features/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
extern "C" {
#include "sqlite3.h"
}
#endif

namespace Native::Sqlite {

TArray<FMemoryItem> SearchRows(DB Database, const TArray<float> &Vector,
                               int32 TopK) {
  TArray<FMemoryItem> Results;
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return !Handle
             ? Results
             : [&]() -> TArray<FMemoryItem> {
                 const int32 Limit = TopK > 0 ? TopK : 10;
                 const char *Sql =
                     "SELECT id, text, type, importance, timestamp, distance "
                     "FROM memories "
                     "WHERE embedding MATCH ? "
                     "ORDER BY distance "
                     "LIMIT ?;";

                 sqlite3_stmt *Statement = nullptr;
                 return sqlite3_prepare_v2(Handle, Sql, -1, &Statement,
                                           nullptr) != SQLITE_OK
                            ? Results
                            : [&]() -> TArray<FMemoryItem> {
                                const FString JsonVector =
                                    CodecAdapters::BuildJsonVector(Vector);
                                sqlite3_bind_text(
                                    Statement, 1, TCHAR_TO_UTF8(*JsonVector),
                                    -1, SQLITE_TRANSIENT);
                                sqlite3_bind_int(Statement, 2, Limit);
                                CodecAdapters::CollectSearchRows(Statement,
                                                                Results);
                                sqlite3_finalize(Statement);
                                return Results;
                              }();
               }();
#else
  (void)Database;
  (void)Vector;
  (void)TopK;
  return Results;
#endif
}

TArray<FMemoryItem> Search(DB Database, const TArray<float> &Vector,
                           int32 TopK) {
  return SearchRows(Database, Vector, TopK);
}

} // namespace Native::Sqlite
